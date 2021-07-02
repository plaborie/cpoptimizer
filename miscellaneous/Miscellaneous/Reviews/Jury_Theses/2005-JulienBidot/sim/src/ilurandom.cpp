// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilurandom.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/ilurandom.h>

//#include <stdlib.h>
//#if defined(ILOUSESTL)
//#include <fstream>
//#include <stdio>
//#else
//#include <fstream.h>
//#include <stdio.h>
//#endif


ILOSTLBEGIN



// --------------------------------------------------------------------------
// CLASS OF RANDOM VARIABLE FOLLOWING A UNIFORM DISTRIBUTION
// TRUNCATED BETWEEN min AND max
// --------------------------------------------------------------------------

IloNum
IluUniformVariableI::getValue(IloNum v) {
	IloNum temp = v + _rand.getFloat()*(_max - v);
	if((temp >= _min)&&(temp <= _max))
		return temp;
	else if(_min == _max)
		return _min;
	else {
		printf("%.14f is out of range!\n");
		return IloInfinity;
	}
}


// --------------------------------------------------------------------------
// CLASS OF RANDOM VARIABLE FOLLOWING A NORMAL DISTRIBUTION
// OF AVERAGE avg, STANDARD DEVIATION sigma AND TRUNCATED BETWEEN min AND max
// --------------------------------------------------------------------------

IloInt  IluNormalVariableI::_nbSteps;
IloNum  IluNormalVariableI::_step;
IloInt  IluNormalVariableI::_k;
IloNum  IluNormalVariableI::_epsilon;
IloNum* IluNormalVariableI::_tab;
IloNum* IluNormalVariableI::_invtab;
IloNum* IluNormalVariableI::_avgs;

IluNormalVariableI::IluNormalVariableI(IloEnv env, IluRandomEnv rand,
									   IloNum min, IloNum max,
									   IloNum avg, IloNum sigma)
	:IluRandomVariableI(env, rand, min, max),
	 _avg    (avg),
	 _sigma  (sigma),
	 _xMin   (0.0),
	 _xMax   (1.0),
	 _deltaX (1.0) {
	if (0 == _invtab) {
		cout << "Uninitialized instance of class IluNormalVariableI: please call IluNormalVariableI::Init()" << endl;
		exit(0);
	}
	computeBounds();
}

void
IluNormalVariableI::Init(IloInt precision, IloInt k, IloNum epsilon) {
	if (0 != _invtab)
		return;
	_nbSteps = precision + 1;
	_k = k;
	IloNum delta = 2 * _k;
	_step = ((IloNum)_nbSteps) / delta;
	_epsilon = epsilon;
	IloInt i,j;
	_invtab = new IloNum[_nbSteps];
	_avgs = new IloNum[_nbSteps];
	IloNum coeff = 1.0/sqrt(2 * IloPi);
	for (i=0; i<_nbSteps; ++i) 
		_invtab[i] = coeff*exp(-0.5 * pow(-_k+(i*delta)/_nbSteps,2));

	// Average  (1.)
	_avgs[_nbSteps-1] =  _invtab[_nbSteps-1] * _k;
	for (i=_nbSteps-2; i>=0; --i) {
		IloNum x = -_k + ((IloNum)i)/_step;
		_avgs[i] = _avgs[i+1] + _invtab[i] * x;
	}

	// Integration	
	_invtab[0] = 0;
	for (i=1; i<_nbSteps; ++i)
		_invtab[i] += _invtab[i-1];
	IloNum sum = _invtab[_nbSteps-1];
	
	// Average	 (2.)
	for (i=0; i<_nbSteps-1; ++i) 
		_avgs[i] = _avgs[i]/(sum - _invtab[i]);
	_avgs[_nbSteps-1] = _k;
	
	// Normalization		
	for (i=1; i<_nbSteps; ++i)
		_invtab[i] = _invtab[i]/sum;

	// Reverse curve	
	_tab = new IloNum[_nbSteps];
	_tab[0] = -_k;
	i=0;
	for (j=1; j<_nbSteps; ++j) {
		IloNum y = (IloNum)j / (IloNum)_nbSteps;
		while ((i < _nbSteps) && (_invtab[i] < y)) 
			i++;
		IloNum r = i - ((_invtab[i] - y) / (_invtab[i] - _invtab[i-1]));
		_tab[j] = - _k + (r*delta)/_nbSteps;
		i--;
	}
}

void
IluNormalVariableI::End() {
	delete [] _tab;
	delete [] _invtab;
	delete [] _avgs;
}

IloNum
IluNormalVariableI::getValue()  {
	IloNum x = (_xMin + (_rand.getFloat()*_deltaX)) * _nbSteps;
	IloInt i = (IloInt)x;
	IloNum z;
	if (i == _nbSteps - 1)
		z =  (1.0 - _tab[i])*(x-i)+_tab[i];
	else
		z = (_tab[i+1]-_tab[i])*(x-i)+_tab[i];
	return _avg + _sigma * z;
}

IloNum
IluNormalVariableI::getValue(IloNum v)  {
	if(0.0 == _sigma)
		return _avg;
	else {
		IloNum w = (v - _avg)/_sigma;
		IloNum xmin;
		if (w - _epsilon < - _k) {
			xmin = 0;
		} else if (w + _epsilon > _k) {
			return v;
		} else {
			xmin = _invtab[(IloInt)((w + _k)*_step)];
		}
		IloNum x = (xmin + (_rand.getFloat()*(1-xmin))) * _nbSteps;
		IloInt i = (IloInt)x;
		IloNum z;
		if (i == _nbSteps - 1)
			z =  (1.0 - _tab[i])*(x-i)+_tab[i];
		else
			z = (_tab[i+1]-_tab[i])*(x-i)+_tab[i];
		return _avg + _sigma * z;
	}
}

void
IluNormalVariableI::computeBounds() {
	if (_min - _epsilon > _avg - _k*_sigma)
		_xMin = _invtab[(IloInt)(_step *(((_min - _avg)/_sigma) + _k))];
	else
		_xMin = 0.0;
	if (_max + _epsilon < _avg + _k*_sigma)
		_xMax = _invtab[(IloInt)(_step *(((_max - _avg)/_sigma) + _k))];
	else
		_xMax = 1.0;
	_deltaX = _xMax - _xMin;
}

void
IluNormalVariableI::setAverage(IloNum avg) {
	_avg = avg;
	computeBounds();
}

void
IluNormalVariableI::setStandardDev(IloNum sigma) {
	_sigma = sigma;
	computeBounds();
}

IloNum
IluNormalVariableI::getAverage(IloNum v) {
	if (v == _max)
		return _max;
	IloNum w = (v - _avg)/_sigma;
	IloInt cell= (IloInt)((w + _k)*_step);
	return IloMin(_max,_avg+(_avgs[cell]*_sigma));
}

IloNum
IluNormalVariableI::getCoverValue(IloNum x) {
	return _max;}

IloNum
IluNormalVariableI::getCoverValue(IloNum v, IloNum x) {
	return _max;}

// --------------------------------------------------------------------------
// CLASS OF RANDOM VARIABLE FOLLOWING DISTRIBUTION FUNCTION GIVEN AS ARGUMENT
// TRUNCATED BETWEEN min AND max
// --------------------------------------------------------------------------

IluFunctionVariableI::IluFunctionVariableI(IloEnv env,
										   IluRandomEnv rand,
										   IloNum min, IloNum max,
										   IloNum (*f)(IloNum),
										   IloInt precision)
	:IluRandomVariableI(env, rand, min, max),
	 _f       (f),
	 _nbSteps (precision+1),
	 _tab     (0),
	 _invtab  (0),
	 _avgs    (0) {
	_invtab = new (_env) IloNum[_nbSteps];
	_tab    = new (_env) IloNum[_nbSteps];	
	_avgs   = new (_env) IloNum[_nbSteps];
	init();
}

void
IluFunctionVariableI::setFunction(IloNum (*f)(IloNum)) {
	_f = f;
	init();
}

void
IluFunctionVariableI::init() {
	IloInt i,j;
	IloNum step = (_max - _min)/_nbSteps;
	_step = step;
	for (i=0; i<_nbSteps; ++i) 
		_invtab[i] = _f(_min + i*_step);
	

	// Average  (1.)
	_avgs[_nbSteps-1] =  _invtab[_nbSteps-1] * _max;
	for (i=_nbSteps-2; i>=0; --i) {
		IloNum x = _min + i*_step;
		_avgs[i] = _avgs[i+1] + _invtab[i] * x;
	}
	
	// Integration	
	_invtab[0] = 0;
	for (i=1; i<_nbSteps; ++i)
		_invtab[i] += _invtab[i-1];
	IloNum sum = _invtab[_nbSteps-1];
	
	// Average	 (2.)
	for (i=0; i<_nbSteps-1; ++i) 
		_avgs[i] = _avgs[i]/(sum - _invtab[i]);
	_avgs[_nbSteps-1] = _max;
	
	// Normalization			
	for (i=1; i<_nbSteps; ++i)
		_invtab[i] = _invtab[i]/sum;
	
	// Reverse curve	
	_tab[0] = _min;
	i=0;
	for (j=1; j<_nbSteps; ++j) {
		IloNum y = (IloNum)j / (IloNum)_nbSteps;
		while ((i < _nbSteps) && (_invtab[i] < y)) 
			i++;
		IloNum r = i - ((_invtab[i] - y) / (_invtab[i] - _invtab[i-1]));
		_tab[j] = _min + (r*_step);
		i--;
	}
}

IloNum
IluFunctionVariableI::getValue() {
	IloNum x = _rand.getFloat() * (IloNum)_nbSteps;
	IloInt i = (IloInt)x;
	IloNum z;
	if (i == _nbSteps - 1)
		z = (1.0 - _tab[i])*(x-i)+_tab[i];
	else
		z = _step*(x-i)+_tab[i];
	return z;
}

IloNum
IluFunctionVariableI::getValue(IloNum v) {
	IloInt cell;
	IloNum xmin;
	cell= (IloInt)(((IloNum)_nbSteps)*(v - _min)/(_max - _min));
	xmin = _invtab[cell];
	IloNum x = (xmin + (_rand.getFloat()*(1-xmin))) * _nbSteps;
	IloInt i = (IloInt)x;
	IloNum z;
	if (i == _nbSteps - 1)
		z =  (1.0 - _tab[i])*(x-i)+_tab[i];
	else
		z = (_tab[i+1]-_tab[i])*(x-i)+_tab[i];
	return z;
}

IloNum
IluFunctionVariableI::getAverage(IloNum v) {
	if (v == _max)
		return _max;
	IloInt cell = (IloInt)(((IloNum)_nbSteps)*(v - _min)/(_max - _min));
	return IloMin(_max,_avgs[cell]);
}

IloNum
IluFunctionVariableI::getCoverValue(IloNum x) {
	if (x > 1)
		printf("Error, x is greater than 1!");
	if (0 == x)
		return _min;
	IloInt cell = 1;
	while (_invtab[cell] < x)
		cell++;
// Linear interpolation
	IloNum y = _tab[cell-1]+(x - _invtab[cell-1])*_step/(_invtab[cell]-_invtab[cell-1]);
	return y;
}

IloNum
IluFunctionVariableI::getCoverValue(IloNum v, IloNum x) {
	if (x > 1)
		printf("Error, x is greater than 1!");
	if (v == _max)
		return _max;
	if (0 == x)
		return IloMax(_min,v);
	IloInt cell = 0;
	while (_tab[cell] < v)
		cell++;
// Linear interpolation
	IloNum surface = _invtab[cell-1]+(v-_tab[cell-1])*(_invtab[cell]-_invtab[cell-1])/_step;
	IloNum y = getCoverValue(x*(1-surface));
	return y;
}

// --------------------------------------------------------------------------
// CLASS OF HISTOGRAM
// RECORDS THE LAST nbMax REALIZATIONS OF A RANDOM VARIABLE AND
// INCREMENTALLY MAINTAINS THE AVERAGE AND STANDARD DEVIATION
// --------------------------------------------------------------------------

IluHistogramI::IluHistogramI(IloEnv env, IloInt nbMax)
	:_env			(env),
	 _nb			(0),
	 _nbMax			(nbMax),
	 _x1			(0.0),
	 _x2			(0.0),
	 _last			(-1),
	 _deterministic	(IloFalse),
	 _next			(0) {
	IloInt size;
	if(IloIntMax == nbMax)
		size = 1;
	else
		size = nbMax;
	_array = new (env) ItemI[size];
	for(IloInt i = 0 ; i < size ; i++)
		_array[i].setValue(IloInfinity, 0);
}

IloNum
IluHistogramI::getStandardDev() const {
	if(IloTrue == _deterministic)
		return 0.0;
	else {
		IloNum avg = getAverage();
		IloNum d2 = (_x2 - 2*avg*_x1 + _nb*avg*avg)/(_nb-1);
		if (d2 > 0)
			return sqrt((_x2 - 2*avg*_x1 + _nb*avg*avg)/(_nb-1));
		else
			return 0.0;
	}
}

void
IluHistogramI::setValue(IloNum val, IloInt seed) {
	if (IloIntMax == _nbMax) {
		_x1 += val;
		_x2 += val * val;
		_nb++;
	} else if (_nb == _nbMax) {
		printf("The histogram is full and cannot store this value.\n");
	} else {
		_x1 += val;
		_x2 += val * val;
		_last += 1;
		_array[_last].setValue(val, seed);
		_nb++;
	}
}

void
IluHistogramI::setValue(IloNum val, IloInt seed, IloInt index) {
	if((index < 0)||(index > _nbMax))
		printf("The value cannot be stored because the index is not correct.\n");
	else if(IloIntMax == _nbMax) {
		_x1 += val;
		_x2 += val * val;
		_nb++;	
	}
	else {
		IloNum temp = 0.0;
		if(_array[index].getValue() < IloInfinity) {
			temp = _array[index].getValue();
			_nb--;
		}
		if(-1 == _last)
			_last = _nbMax - 1;
		_array[index].setValue(val, seed);
		_x1 = _x1 + val - temp;
		_x2 -= temp * temp;
		_x2 += val * val;
		_nb++;
	}
}

void
IluHistogramI::clean() {
	_nb    = 0;
	_x1    = 0.0;
	_x2    = 0.0;
	_last  = -1;
	for(IloInt i = 0 ; i < _nbMax ; i++)
		_array[i].setValue(IloInfinity, 0);
}


void
IluHistogramI::display(const char* fname, IloInt nbPoints) {	
	ofstream out(fname);
	IloInt i;
	out << "# Average = \t " << getAverage() << endl;
	out << "# Standard deviation = \t " << getStandardDev() << endl;
	if (_nbMax < IloIntMax) {
		if(IloTrue == _deterministic)
			for (i = 0 ; i < nbPoints ; ++i)
				out << _array[0].getValue() << "\t" << 1.0/(IloNum)nbPoints << endl;
		else {
			IloNum min = +IloInfinity;
			IloNum max = -IloInfinity;
			for (i = 0 ; i < _last + 1 ; i++) {
				IloNum val = _array[i].getValue();
				if (val < min)
					min = val;
				if (val > max)
					max = val;
			}
			IloInt* tab = new IloInt[nbPoints+1];
			IloNum delta = (max - min)/nbPoints;
			for (i = 0 ; i < nbPoints ; ++i)
				tab[i]=0;
	// For smoothing the curve: it is particularly suited when the curve is regular as a curve representing a normal law.
			for (i = 0 ; i < _last + 1 ; i++) {
				IloInt id = (IloInt)((_array[i].getValue()-min)/delta);
				tab[id]++;
				if (0 < id)
					tab[id-1]++;
				if (id < nbPoints-1)
					tab[id+1]++;
			}
			for (i = 0 ; i < nbPoints ; ++i)
				out << min + i*delta << "\t" << (IloNum)tab[i]/(IloNum)(3*_nb) << endl;
			delete [] tab;
		}
		out.close();
	}
}


MyRandom::MyRandom(IloSolver solver, IloInt seed)
	:_solver(solver), _lastStamp(0) {
	IlcManager manager = solver.getManager();
	_seed.setValue(manager, seed);
	IloEnv env = solver.getEnv();
	_rand = IloRandom(env, seed);
}


IloNum
MyRandom::getFloat() {
	testStampAndReSeed();
	IloNum val = _rand.getFloat();
	return val;
}


IloInt
MyRandom::getInt(IloInt maxVal) {
	testStampAndReSeed();
	return _rand.getInt(maxVal);
}


void
MyRandom::init(IloInt seed) {
	IlcManager manager = _solver.getManager();
	_seed.setValue(manager, seed);
	_rand.reSeed(seed);
	_lastStamp = 0;
}


void
MyRandom::testStampAndReSeed() {
	IlcManager manager = _solver.getManager();
	if(manager.getStamp() != _lastStamp) {
		_lastStamp = manager.getStamp();
		_rand.reSeed(_seed);
		_seed.setValue(manager, _rand.getInt(IloIntMax));
	}
}