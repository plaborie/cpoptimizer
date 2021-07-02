// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the IluRandomVariableI class, the IluUniformVariableI
// class, the IluNormalVariableI class, the IluFunctionVariableI class,
// the IluHistogramI class and the MyRandom class.

#ifndef __SIM_ilurandomsimH
#define __SIM_ilurandomsimH

//#include<ilconcert/iloenv.h>
//#include<ilconcert/ilorandom.h>
#include<ilsolver/ilosolver.h>

typedef IloRandom IluRandomEnv;

// --------------------------------------------------------------------------
// ROOT CLASS FOR RANDOM VARIABLES
// THIS CLASS PREMITS US TO BUILD RANDOM VARIABLES WHOSE DOMAINS ARE FINITE
// AND BOUNDED BY min AND max.
// --------------------------------------------------------------------------
class IluRandomVariableI {
protected:
	IloEnv				_env;
	IloNum				_min;
	IloNum				_max;
	IluRandomEnv		_rand;
	IluRandomVariableI*	_next;	// next is used to recycle some random variables.
public:
	IluRandomVariableI(IloEnv env, IluRandomEnv rand, IloNum min, IloNum max)
		:_env(env), _rand(rand), _min(min), _max(max), _next(0) {}
	~IluRandomVariableI(){}
	IloNum getMin() const { return _min; }
	IloNum getMax() const { return _max; }	
	IluRandomEnv getRandom() const { return _rand; }
	IloEnv getEnv() const { return _env; }
	IluRandomVariableI* getNext() const { return _next; }
	void setNext(IluRandomVariableI* next) { _next = next; }
	virtual IloNum getValue() = 0;
	/* Returns a realization of the random variable. */ 
	virtual IloNum getValue(IloNum v) = 0;
	/* Returns a realization of the random variable that is greater or
     equal to v. The distribution law is truncated and
     renormalized. */
	virtual IloNum getAverage() = 0;
	/* Returns the average of the random variable. */ 
	virtual IloNum getAverage(IloNum v) = 0;
	/* Returns the average of the random variable for the part of the
     distribution that is greater than or equal to v. The distribution law
     is truncated and renormalized. */
	virtual IloNum getCoverValue(IloNum x) = 0;
	/* Returns the value of the random variable for the part of the
	distribution that is covered by x*100 percent. */
	virtual IloNum getCoverValue(IloNum v, IloNum x) = 0;
	/* Returns the value of the random variable for the part of the
	distribution that is greater than or equal to v and covered by x*100 percent.
	The distribution law is truncated and renormalized. */

	class IluException :public IloException {
	public:
		IluException (const char* msg):IloException(msg){}
		~IluException (){}
	};
protected:
	void setMin(IloNum min) {
		_min = min; }
	void setMax(IloNum max) {
		_max = max; }	
};

// --------------------------------------------------------------------------
// CLASS OF RANDOM VARIABLE FOLLOWING A UNIFORM DISTRIBUTION
// BETWEEN min AND max
// --------------------------------------------------------------------------
class IluUniformVariableI :public IluRandomVariableI {
public:
	IluUniformVariableI(IloEnv env, IluRandomEnv rand,
											IloNum min, IloNum max)
		:IluRandomVariableI(env, rand, min, max){}
	~IluUniformVariableI(){}
	IloNum getValue() {	return _min + _rand.getFloat()*(_max - _min); }
	IloNum getValue(IloNum v);
	IloNum getAverage() { return (_min + _max)/2.0; }
	IloNum getAverage(IloNum v) { return (v + _max)/2.0; }
	IloNum getCoverValue(IloNum x) { return x * (_max - _min) + _min; }
	IloNum getCoverValue(IloNum v, IloNum x) { return x * (_max - v) + v; }
};

// --------------------------------------------------------------------------
// CLASS OF RANDOM VARIABLE FOLLOWING A NORMAL DISTRIBUTION
// OF AVERAGE avg, STANDARD DEVIATION sigma AND TRUNCATED BETWEEN min AND max
// --------------------------------------------------------------------------
class IluNormalVariableI :public IluRandomVariableI {
private:
	IloNum         _avg;
	IloNum         _sigma;
	IloNum         _xMin;		
	IloNum         _xMax;
	IloNum         _deltaX;
	static IloInt  _nbSteps;
	static IloNum  _step;
	static IloInt  _k;
	static IloNum  _epsilon;
	static IloNum* _tab;
	static IloNum* _invtab;
	static IloNum* _avgs;
public:
	IluNormalVariableI(IloEnv env,
					   IluRandomEnv rand,
					   IloNum min, IloNum max,
					   IloNum avg, IloNum sigma);
	~IluNormalVariableI(){}
	IloNum getStandardDev() const {	return _sigma; }
	void setAverage(IloNum);
	void setStandardDev(IloNum);
	static void Init(IloInt precision = 10000, IloInt k = 5, IloNum epsilon =1e-6);
	static void End();
	IloNum getValue();
	IloNum getValue(IloNum);
	IloNum getAverage() { return _avg; }
	IloNum getAverage(IloNum);
	IloNum getCoverValue(IloNum);
	IloNum getCoverValue(IloNum, IloNum);
	
private:
	void computeBounds();
};

// --------------------------------------------------------------------------
// CLASS OF RANDOM VARIABLE FOLLOWING DISTRIBUTION FUNCTION GIVEN AS ARGUMENT
// TRUNCATED BETWEEN min AND max
// --------------------------------------------------------------------------
class IluFunctionVariableI :public IluRandomVariableI {
private:
	IloNum (*_f)(IloNum);
	IloInt  _nbSteps;
	IloNum* _tab;
	IloNum* _invtab;
	IloNum*	_avgs;
	IloNum	_step;
public:
	IluFunctionVariableI(IloEnv env,
						 IluRandomEnv rand,
						 IloNum min, IloNum max,
						 IloNum (*f)(IloNum),
						 IloInt precision = 10000); // f is a pointer to a function.
	~IluFunctionVariableI() {}
	void setFunction(IloNum (*f)(IloNum));
	IloNum getValue();
	IloNum getValue(IloNum);
	IloNum getAverage() { return _avgs[0]; }
	IloNum getAverage(IloNum);
	IloNum getCoverValue(IloNum);
	IloNum getCoverValue(IloNum, IloNum);
private:
	void init();
};

// --------------------------------------------------------------------------
// CLASS OF HISTOGRAM
// RECORDS THE LAST nbMax REALIZATIONS OF A RANDOM VARIABLE AND
// INCREMENTALLY MAINTAINS THE AVERAGE AND STANDARD DEVIATION.
// THIS CLASS ALSO RECORDS nbMax DIFFERENT SEEDS, EACH SEED CORRESPONDING TO
// A REALIZATION. THESE SEEDS ARE RANDOMLY PICKED.
// --------------------------------------------------------------------------
class IluHistogramI {
public:
	class ItemI {
	private:
		IloNum _val;
		IloInt _seed;		// _seed is picked in [1,IloIntMax] when we construct the histogram.
	public:
		ItemI(IloNum val, IloInt seed)
			:_val(val), _seed(seed) {}
		ItemI()
			:_val(0.0), _seed(1) {}
		~ItemI(){};
		void setValue(IloNum val, IloInt seed) { _val = val; _seed = seed; }
		IloNum getValue() const { return _val; }
		IloInt getSeed() const { return _seed; }
		void init(IloNum val, IloInt seed) { _val = val; _seed = seed; }
	};

private:
	IloEnv	_env;
	IloInt	_nb;
	IloInt	_nbMax;				// _nbMax is the maximum number of realizations that can be stored in the histogram.
								// This is also the size of _array.
	ItemI*	_array;				// _array contains all the realizations.
	IloInt	_last;				// _last represents the index of the cell of _array that contains the last realization.
	IloNum	_x1;				// _x1 is equal to the sum of the realizations.
	IloNum	_x2;				// _x2 is equal to the sum of the square realizations.
	IloBool	_deterministic;		// _deterministic is equal to IloTrue if the standard deviation is equal to zero, otherwise it is equal to IloFalse.
								// In other words _deterministic is equal to IloTrue when all realizations are equal to each other.
	IluHistogramI*	_next;		// _next points to the next histogram; it is used to recycle histograms.

public:
	IluHistogramI(IloEnv env, IloInt nbMax = IloIntMax);	// nbMax is the maximum number of realizations that can be stored in the histogram.
															// if nbMax is equal to IloIntMax, then the histogram can store an unlimited number of realizations.
	~IluHistogramI() {}
	void setValue(IloNum val, IloInt seed);
	void setValue(IloNum val, IloInt seed, IloInt index);
	void clean();
	IloNum getAverage() const {	if(IloTrue == _deterministic) return _array[0].getValue(); else return _x1 / _nb; }
	IloNum getStandardDev() const;
	void display(const char*, IloInt nbPoints = 1000);
	IloNum getValue(IloInt id) const { if(IloTrue == _deterministic) return _array[0].getValue(); else return _array[id].getValue(); }
	IloInt getSeed(IloInt id) const { return _array[id].getSeed(); }
	IloInt getNbSimulations() const { return _nbMax; }
	IloBool isDeterministic() const { return _deterministic; }
	void setDeterministic(IloBool deterministic) { _deterministic = deterministic; }
	IluHistogramI* getNext() const { return _next; }
	void setNext(IluHistogramI* next) { _next = next; }
};



// --------------------------------------------------------------------------
// CLASS OF RANDOM
// RESEEDS _rand WITH _seed WHEN _lastStamp IS DIFFERENT FROM THE CURRENT STAMP.
// --------------------------------------------------------------------------
class MyRandom {
private:
	IluRandomEnv	_rand;
	IlcStamp		_lastStamp;
	IlcRevInt		_seed;
	IloSolver		_solver;

public:
	MyRandom(IloSolver solver, IloInt seed = 1);
	~MyRandom() {}
	IloNum getFloat();
	IloInt getInt(IloInt maxVal);
	void init(IloInt seed);
	void testStampAndReSeed();
	void setSolver(IloSolver solver) { _solver = solver; }
};



#endif
