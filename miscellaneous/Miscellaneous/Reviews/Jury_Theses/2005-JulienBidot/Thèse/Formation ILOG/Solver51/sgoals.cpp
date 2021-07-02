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

ILOCPGOAL0(G2)
{
	IloSolver solver = getSolver();
	IloEnv e = solver.getEnv();
	e.out() << "Goal is G2" << endl;
	return 0;
}

ILOCPGOAL0(G3)
{
	IloSolver solver = getSolver();
	IloEnv e = solver.getEnv();
	e.out() << "Goal is G3" << endl;
	return 0;
}


ILOCPGOAL0(G1)
{
	IloSolver solver = getSolver();
	IloEnv e = solver.getEnv();
	e.out() << "Goal is G1" << endl;
	return IloAndGoal(solver, G2(solver), G3(solver));
}

int main() {
  IloEnv env;
  try {
	IloModel model(env);

    // search for a solution
    // ---------------------
    IloSolver solver(model);
	if (solver.solve(G1(env)))
	{
		env.out() << "Solution : T1 = ";
	}
	else env.out() << "NO SOLUTION" << endl;

	solver.printInformation();

	}
	catch (IloException& ex) {
		cerr << "Error: " << ex << endl;
	} 
	env.end();
	return 0;
}



/*
// --------------------------------------------------------------------------
// define a new ILOG Solver goal "instantiate" called MyMACROIlcInstantiate
// 1 - with the macro ILCGOAL
// --------------------------------------------------------------------------

ILCGOAL1(MyMACROIlcInstantiate, IlcIntVar, x)
{
	if (x.isBound()) return 0;

	IlcInt v = x.getMin();
	return IlcOr(IlcSetValue(x,v), IlcAnd(IlcRemoveValue(x,v), this));
}

// --------------------------------------------------------------------------
// define the ILOG Concert interface called MyMACROIloInstantiate1
// 1 - with the macro ILOCPGOAL
// --------------------------------------------------------------------------

ILOCPGOAL1(MyMACROIloInstantiate1, IloNumVar, x)
{
	IloSolver solver = getSolver();
	IlcIntVar var_x = solver.getIntVar(x);
	return MyMACROIlcInstantiate(solver, var_x);
}

// --------------------------------------------------------------------------
// define a new ILOG Solver goal "instantiate" called MyEXPLICITEIlcInstantiate
// 2 - with subclassing (explicitly)
// --------------------------------------------------------------------------
class MyEXPLICITEIlcInstantiateI : public IlcGoalI {
	IlcIntVar _var;
	public:
		MyEXPLICITEIlcInstantiateI(IloSolver, IlcIntVar);
		IlcGoal execute();
};

MyEXPLICITEIlcInstantiateI::MyEXPLICITEIlcInstantiateI(IloSolver s, IlcIntVar v) :
IlcGoalI(s), _var(v) {}

IlcGoal MyEXPLICITEIlcInstantiateI::execute() {
	if (_var.isBound()) 
		return 0;
	IlcInt v = _var.getMin();
	return IlcOr(IlcSetValue(_var,v), IlcAnd(IlcRemoveValue(_var,v), this));
}

IlcGoal MyEXPLICITEIlcInstantiate(IloSolver s, IlcIntVar v) {
	return new (s.getHeap()) MyEXPLICITEIlcInstantiateI(s, v);
}

// --------------------------------------------------------------------------
// define the ILOG Concert interface called MyMACROIloInstantiate2
// 3 - with the macro ILOCPGOAL
// --------------------------------------------------------------------------
ILOCPGOAL1(MyMACROIloInstantiate2, IloNumVar, x)
{
	IloSolver solver = getSolver();
	IlcIntVar var_x = solver.getIntVar(x);
	return MyEXPLICITEIlcInstantiate(solver, var_x);
}

// --------------------------------------------------------------------------
// define the ILOG Concert interface called MyEXPLICITEIloInstantiate
// 4 - with subclassing (explicitly)
// --------------------------------------------------------------------------
class MyEXPLICITEIloInstantiateI : public IloGoalI {
	IloNumVarI* _var;
	public:
		MyEXPLICITEIloInstantiateI(IloEnvI*, IloNumVarI*);
		virtual IlcGoal extract(const IloSolver) const;
		virtual IloGoalI* makeClone(IloEnvI* env) const;
		virtual void display(ILOSTD(ostream&)) const {};
};

MyEXPLICITEIloInstantiateI::MyEXPLICITEIloInstantiateI(IloEnvI* e, IloNumVarI* v) :
IloGoalI(e), _var(v) {}

IloGoalI* MyEXPLICITEIloInstantiateI::makeClone(IloEnvI* env) const {
	return new(env) MyEXPLICITEIloInstantiateI(env, (IloNumVarI*)_var->makeClone(env));
}

IlcGoal MyEXPLICITEIloInstantiateI::extract(const IloSolver solver) const {
	return MyEXPLICITEIlcInstantiate(solver, solver.getIntVar(_var));
}

IloGoal MyEXPLICITEIloInstantiate(const IloEnv env, const IloNumVar v) {
	return new (env) MyEXPLICITEIloInstantiateI(env.getImpl(), v.getImpl());
}

// --------------------------------------------------------------------------
// define a new ILOG Solver goal "generate" called MyMACROIlcGenerate
// 5 - with the macro ILCGOAL
// --------------------------------------------------------------------------

ILCGOAL1(MyMACROIlcGenerate, IlcIntVarArray, a)
{
	IloSolver s = getSolver();
	IlcInt i = 0;
	IlcInt size = a.getSize();
	while((i < size) && (a[i].isBound()))
		i++;

	if (i == size) 
		return 0;
	else
		return IlcAnd(MyMACROIlcInstantiate(s, a[i]), this);
}

// --------------------------------------------------------------------------
// define a new ILOG Solver goal "instantiate" called MyEXPLICITEIlcGenerate
// 6 - with subclassing (explicitly)
// --------------------------------------------------------------------------
class MyEXPLICITEIlcGenerateI : public IlcGoalI {
	IlcIntVarArray _t;
	public:
		MyEXPLICITEIlcGenerateI(IloSolver, IlcIntVarArray);
		IlcGoal execute();
};

MyEXPLICITEIlcGenerateI::MyEXPLICITEIlcGenerateI(IloSolver s, IlcIntVarArray t) :
IlcGoalI(s), _t(t) {}

IlcGoal MyEXPLICITEIlcGenerateI::execute() {
	IloSolver s = getSolver();
	IlcInt i = 0;
	IlcInt size = _t.getSize();
	while((i < size) && (_t[i].isBound()))
		i++;

	if (i == size) 
		return 0;
	else
		return IlcAnd(MyEXPLICITEIlcInstantiate(s, _t[i]), this);
}

IlcGoal MyEXPLICITEIlcGenerate(IloSolver s, IlcIntVarArray t) {
	return new (s.getHeap()) MyEXPLICITEIlcGenerateI(s, t);
}

// --------------------------------------------------------------------------
// define the ILOG Concert interface called MyMACROIloGenerate1
// 7 - with the macro ILOCPGOAL calling MyMACROIlcGenerate
// --------------------------------------------------------------------------

ILOCPGOAL1(MyMACROIloGenerate1, IloNumVarArray, a)
{
	IloSolver solver = getSolver();
	IlcIntVarArray var_a = solver.getIntVarArray(a);
	return MyMACROIlcGenerate(solver, var_a);
}

// --------------------------------------------------------------------------
// define the ILOG Concert interface called MyMACROIloGenerate2
// 8 - with the macro ILOCPGOAL calling MyEXPLICITEIlcGenerate
// --------------------------------------------------------------------------

ILOCPGOAL1(MyMACROIloGenerate2, IloNumVarArray, a)
{
	IloSolver solver = getSolver();
	IlcIntVarArray var_a = solver.getIntVarArray(a);
	return MyEXPLICITEIlcGenerate(solver, var_a);
}

int main() {
  IloEnv env;
  try {
	IloModel model(env);

    // declare the variables
    // ---------------------
    IloNumVarArray T1(env, 3, 1, 10, ILOINT);

    // post the constraints
    // --------------------
	model.add(T1[0] >= 5);
	model.add(T1[2] <= T1[1]);
	model.add(T1[1] >= 1);

    // search for a solution
    // ---------------------
    IloSolver solver(model);
	if (solver.solve(MyMACROIloGenerate2(env, T1)))
	{
		env.out() << "Solution : T1 = ";
		for(IloInt i = 0; i < T1.getSize(); i++)
			env.out() << solver.getValue(T1[i]) << " - ";
		env.out() << endl;
	}
	else env.out() << "NO SOLUTION" << endl;

	solver.printInformation();

	}
	catch (IloException& ex) {
		cerr << "Error: " << ex << endl;
	} 
	env.end();
	return 0;
}
*/