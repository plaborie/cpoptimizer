// -------------------------------------------------------------- -*- C++ -*-
// File: ./src/iloext/ilonumfunci.cpp
// --------------------------------------------------------------------------
// IBM Confidential
// OCO Source Materials
//
// 5725-A06 5725-A29 5724-Y47 5724-Y48 5724-Y49 5724-Y54 5724-Y55
// Copyright IBM Corp. 2000, 2012
//
// The source code for this program is not published or otherwise
// divested of its trade secrets, irrespective of what has
// been deposited with the U.S. Copyright Office.
// ---------------------------------------------------------------------------

#include <ilconcert/ilonumfunc.h>
#include <ilconcert/ilosegfunc.h>

#define EPSILON (1e-6)

ILOSTLBEGIN

//------------------------------------------------------------
// IloFunctionSegmentI
//------------------------------------------------------------

IloFunctionSegmentI::IloFunctionSegmentI()
  : IloSkipListNodeI (),
    _valueRef        (0), 
    _slope           (0)
{}

IloFunctionSegmentI::IloFunctionSegmentI(IloEnvI* env, 
                                         IloInt level)  
  : IloSkipListNodeI (env, level),
    _valueRef        (0), 
    _slope           (0)
{}

#ifdef ILO_SEGF_GENERIC_ALLOC
IloFunctionSegmentI::IloFunctionSegmentI(IloMemoryManagerI* env, 
                                         IloInt level)  
  : IloSkipListNodeI (env, level),
    _valueRef        (0), 
    _slope           (0)
{}
#endif


void IloFunctionSegmentI::setSameAs(const IloSkipListNodeI* segment)
{
  IloFunctionSegmentI* rseg = (IloFunctionSegmentI*)segment;
  _valueRef        = rseg->_valueRef;
  _slope           = rseg->_slope;
}

#define IloAreAlmostEqual(val1, val2)		\
  (IloAbs(val2-val1) <= 1e-6)

#define IloIsAlmostNull(val)			\
  (IloAbs(val) <= 1e-6)

IloBool  
IloFunctionSegmentI::isSameAs(IloNum x, 
                              IloNum val, 
                              IloNum slope) const {
  if (((val == IloInfinity) && (_valueRef == IloInfinity)) ||
      ((val ==-IloInfinity) && (_valueRef ==-IloInfinity)))
    return IloTrue;

  if (IloIsAlmostNull(_slope) && IloIsAlmostNull(slope))
    return IloAreAlmostEqual(_valueRef, val);

  if (!IloAreAlmostEqual(slope, _slope))
    return IloFalse;

  if (_start != - IloInfinity) {
    const IloNum v = _valueRef + (x - _start)*_slope;
    return IloAreAlmostEqual(v, val);
  } else {
    const IloNum v = _valueRef + (x - getXRight())*_slope;
    return IloAreAlmostEqual(v, val);
  }
}

IloNum IloFunctionSegmentI::getValue(IloNum x) const
{
  if (0 == _slope){
    return _valueRef;
  } else if (getXLeft() != -IloInfinity){
    return (getValueLeft() + ((x - getXLeft()) * _slope));
  } else {
    IloFunctionSegmentI* next =
      (IloFunctionSegmentI*) getNext();
    //    return (next->getValueLeft() - (((next->getXLeft())-x)*_slope));
    return (_valueRef + (((next->getXLeft())-x)*(-_slope)));
  }
}

IloNum IloFunctionSegmentI::getValueLeft() const
{
  if (0 == _slope) return _valueRef;

  if (getXLeft() == - IloInfinity){
    if (_slope > 0) return - IloInfinity;
    else {
      assert (_slope < 0);
      return + IloInfinity;
    }
  }
  return _valueRef;
}

IloNum IloFunctionSegmentI::getValueRight() const
{
  if ((0 == _slope) || (getXLeft() == - IloInfinity)) {
    return _valueRef;
  } 
  return _valueRef + (getXRight() - getXLeft()) * _slope;
}

IloNum IloFunctionSegmentI::getMin() const
{
  if (_slope >= 0) return getValueLeft();
  return getValueRight();
}

IloNum IloFunctionSegmentI::getMax() const
{
  if (_slope <= 0) return getValueLeft();
  return getValueRight();
}

IloNum IloFunctionSegmentI::getArea() const
{
  return (getXRight()-getXLeft())*
    (getValueLeft()+getValueRight())/2;
}

IloBool
IloFunctionSegmentI::minInterval(IloNum x1, IloNum x2, IloNum val, IloNum slope, 
                                 IloNum& ax1, IloNum& ax2) {
  register IloNum xL = getXLeft();
  register IloNum xR = getXRight();
  register IloNum rx1 = (x1 < xL) ? xL : x1;
  register IloNum rx2 = (x2 > xR) ? xR : x2;
  if (slope == _slope) {
    // parallel curves
    if (getValue(x1) > val) {
      ax1 = rx1;
      ax2 = rx2;
      return IloTrue;
    } else return IloFalse;
  } else {
    register IloNum vL = _valueRef;
    if (x1 == -IloInfinity) x1 = x2;
    if (xL == -IloInfinity) xL = xR;
    register IloNum x 
      = (val - vL - slope * x1 + _slope * xL) / (_slope - slope);
    if (slope < _slope) {
      if (x < rx2) {
        ax1 = IloMax(x, rx1);
        ax2 = rx2;
        return IloTrue;
      } else return IloFalse;
    } else {
      if (x > rx1) {
        ax1 = rx1;
        ax2 = IloMin(x, rx2);
        return IloTrue;
      } else return IloFalse;
    }
  }
}

IloBool
IloFunctionSegmentI::maxInterval(IloNum x1, IloNum x2, IloNum val, IloNum slope, 
                                 IloNum& ax1, IloNum& ax2) {
  register IloNum xL = getXLeft();
  register IloNum xR = getXRight();
  register IloNum rx1 = (x1 < xL) ? xL : x1;
  register IloNum rx2 = (x2 > xR) ? xR : x2;
  if (slope == _slope) {
    // parallel curves
    if (getValue(x1) < val) {
      ax1 = rx1;
      ax2 = rx2;
      return IloTrue;
    } else return IloFalse;
  } else {
    register IloNum vL = _valueRef;
    if (x1 == -IloInfinity) x1 = x2;
    if (xL == -IloInfinity) xL = xR;
    if (slope == 0) x1 = 0;
    if (_slope == 0) xL = 0;
    register IloNum x 
      = (val - vL - slope * x1 + _slope * xL) / (_slope - slope);
    if (slope > _slope) {
      if (x < rx2) {
        ax1 = IloMax(x, rx1);
        ax2 = rx2;
        return IloTrue;
      } else return IloFalse;
    } else {
      if (x > rx1) {
        ax1 = rx1;
        ax2 = IloMin(x, rx2);
        return IloTrue;
      } else return IloFalse;
    }
  }
}

void IloFunctionSegmentI::display(ILOSTD(ostream)& out) const
{
  out << "[" << getXLeft() << ".." <<  getXRight() << "):" 
      << getValueLeft();
  if ((getSlope()!=0.0) && 
      (getValueLeft()!=IloInfinity) && (getValueLeft()!=-IloInfinity)) 
    out << "(" << getSlope() << ")";
}

//------------------------------------------------------------
// IloSegmentedFunctionI
//------------------------------------------------------------

#ifndef ILO_SEGF_GENERIC_ALLOC
IloSegmentedFunctionI::IloSegmentedFunctionI(IloEnvI* env, 
                                             IloNum xMin, 
                                             IloNum xMax,
                                             IloNum dvalue,
                                             const char* name)
  : IloSkipListI    (env, name)
  , _xMin           (xMin)
  , _xMax           (xMax)
  , _xRefPoint      (0)
  , _vRefPoint      (0)
  , _valuesComputed (IloTrue)
  , _cursor         (0)
  , _intervals      (0)
{
  init();
  getBeginSegment()->setValueRef(dvalue);
}
#else
IloSegmentedFunctionI::IloSegmentedFunctionI(IloEnvI* env, 
                                             IloNum xMin, 
                                             IloNum xMax,
                                             IloNum dvalue,
                                             const char* name)
  : IloSkipListI    (env->getGeneralAllocator(), env->getRandom(), name)
  , _xMin           (xMin)
  , _xMax           (xMax)
  , _xRefPoint      (0)
  , _vRefPoint      (0)
  , _valuesComputed (IloTrue)
  , _cursor         (0)
  , _intervals      (0)
{
  init();
  getBeginSegment()->setValueRef(dvalue);
}

IloSegmentedFunctionI::IloSegmentedFunctionI(IloMemoryManagerI* env, 
                                             IloNum xMin, 
                                             IloNum xMax,
                                             IloNum dvalue,
					     IloRandomI* random,
                                             const char* name)
  : IloSkipListI    (env, random, name)
  , _xMin           (xMin)
  , _xMax           (xMax)
  , _xRefPoint      (0)
  , _vRefPoint      (0)
  , _valuesComputed (IloTrue)
  , _cursor         (0)
  , _intervals      (0)
{
  init();
  getBeginSegment()->setValueRef(dvalue);
}

IloSegmentedFunctionI* IloSegmentedFunctionI::makeClone(IloMemoryManagerI* mgr) const {
//TO CHECK	notifyObservers();
  IloSegmentedFunctionI* c = new(mgr) IloSegmentedFunctionI(mgr, -IloInfinity, IloInfinity, 0.0, _random, getName());
  c->copy(this);
  return c;
}
#endif

void IloSegmentedFunctionI::ensureIntervals() {
  if (0 == _intervals)
    _intervals = new (getEnv()) IloIntervalListI(getEnv(), -IloInfinity, IloInfinity);
}

IloSegmentedFunctionI::~IloSegmentedFunctionI() {
#if !defined(NO_END_DELETER)
  if (_intervals) 
    delete _intervals;
#endif
  destroyAll();
}

IloParameterI* IloSegmentedFunctionI::makeCopy() const {

  IloSegmentedFunctionI* fct = new (getEnv())
    IloSegmentedFunctionI(getEnv(),
                          _xMin,
                          _xMax, 
                          getBeginSegment()->getValueRef(),
                          getName());

  fct->copy(this);
  return (IloParameterI*)fct;
}

IloBool IloSegmentedFunctionI::isSemiConvex() {
  // Basically, function is not semi-convex if it has a local maxima.
  // I.e. there is an interval where function is increasing and then it gets
  // decreasing. So we iterate over x axis from left to right and maintain
  // increasing status. Once increasing flag is set to true and we found an
  // interval where function is decreasing we return false.
  const IloNum epsilon = 1e-6;
  IloBool increasing = IloFalse;
  const IloNum xmin = _xMin;
  const IloNum xmax = _xMax;
  IloFunctionSegmentI* csor = (IloFunctionSegmentI*)(searchSlice0(xmin));
  IloNum vprev = csor->getValueLeft();
  for(; csor->getStart() < xmax; csor = (IloFunctionSegmentI*)csor->getNext()) {
    const IloNum vleft  = csor->getValueLeft();
    if (vprev + epsilon < vleft) {
      increasing = IloTrue;
    } else if (vprev > vleft + epsilon) {
      if (increasing)
        return IloFalse;
    }
    IloNum xright = csor->getXRight();
    IloNum vright = csor->getValueRight();
    if (xright > xmax) {
      xright = xmax;
      vright = csor->getValue(xmax);
    }
    if (vleft + epsilon < vright)
      increasing = IloTrue;
    else if (vleft > vright + epsilon) {
      if (increasing)
        return IloFalse;
    }
    vprev = vright;
  }
  return IloTrue;
}

IloInt IloSegmentedFunctionI::countSegments() {
  IloInt count = 0;
  IloFunctionSegmentI* p  = getBeginSegment();  
  while (!p->isTail()) {
    p = (IloFunctionSegmentI*)p->getNext();
    count++;
  }
  return count;
}

void IloSegmentedFunctionI::setReferenceValue(IloNum x0, IloNum v0){
  assert(x0 >= getXMin());
  assert(x0 <  getXMax());
  _xRefPoint = x0;
  _vRefPoint = v0;
  notifyChange();
}

void
IloSegmentedFunctionI::copy(const IloSegmentedFunctionI* fct) {
  // clear & init interval
  reset();
  _xMin = fct->_xMin;
  _xMax = fct->_xMax;
  
  // copy function segments
  IloFunctionSegmentI* curr = fct->getBeginSegment();  
  IloFunctionSegmentI* p  = getBeginSegment();  
  p->setSameAs(curr);
  curr = (IloFunctionSegmentI*)curr->getNext();
  while (!curr->isTail()) {
    p = (IloFunctionSegmentI*)insertLast( curr->getStart() );
    p->setSameAs(curr);
    curr = (IloFunctionSegmentI*)curr->getNext();
  }

  // specific inits
  _xRefPoint      = fct->_xRefPoint;
  _vRefPoint      = fct->_vRefPoint;
  _valuesComputed = fct->_valuesComputed;
}
     
void IloSegmentedFunctionI::setSteps(IloNumArray x, 
                                     IloNumArray v)
{
  assert (x.getSize()+1 == v.getSize());

  // Clean the function
  reset();
  
  IloNum prevx = _xMin;  
  
  IloInt iSize = x.getSize();
  IloInt i =0;

  while (i < iSize) {
    // skip bad data
    if ((x[i] > prevx) && (x[i] < _xMax)){
      doSetValue(prevx, x[i], v[i], 0.0);
      prevx = x[i];
    }
    ++i;
  }
  
  if (prevx < _xMax)
    doSetValue(prevx, _xMax, v[iSize], 0.0);
  notifyChange();
}

void IloSegmentedFunctionI::setSteps(IloNum yl,
				     IloNumArray x, 
                                     IloNumArray v)
{
  assert (x.getSize() == v.getSize());
  // Clean the function
  reset();
  const IloInt n = x.getSize();
  if ((0==n) || (_xMax<=x[0])) {
    doSetValue(_xMin, _xMax, yl, 0.0);
  } else {
    doSetValue(_xMin, x[0], yl, 0.0);
    IloInt i;
    for (i=1; i<n; ++i) {   
      if (x[i]<x[i-1]) {
	throw(Exception("Bad array for step function"));
      } else if (x[i-1]<x[i]) {
	if (x[i]<_xMax) {
	  doSetValue(x[i-1],x[i],v[i-1],0.0);
	} else {
	  doSetValue(x[i-1],_xMax,v[i-1],0.0);	  
	  break;
	}
      }
    }
    if (i == n) {
      doSetValue(x[n-1],_xMax,v[n-1],0.0);	  
    }
  }
  notifyChange();
}


IloNum IloGetDeltaSegmentedFunction(IloNum x0,
				    IloNumArray x, 
				    IloNumArray s) {
  // TODO: Avoid quadratic iteration on function steps in
  // definePiecewiseLinear
  IloNum  delta = 0.0;
  IloBool slopePassed = IloFalse;
  for (IloInt i=0; i<x.getSize(); ++i) {
    if (x0 == x[i]) {
      if (!slopePassed) {
	slopePassed = IloTrue;
      } else {
	delta += s[i];
      }
    } else if (x0 < x[i]) {
      break;
    }
  }
  return delta;
}

void IloSegmentedFunctionI::definePiecewiseLinear(IloNumArray x, 
						  IloNumArray s,
						  IloNum x0, 
						  IloNum y0)
{
  assert (x.getSize()+1 == s.getSize());
  IloInt n = x.getSize();
  // Clean the function
  reset();
  _valuesComputed = IloFalse;
  IloInt i =0;

  // Skip discontinuities to only create segments with slopes
  IloFunctionSegmentI* p = (IloFunctionSegmentI*)_header;
  p->_slope = s[0];
  IloNum prevx = x[0];
  while (i < n) {
    assert((i==n-1)||(prevx <= x[i+1]));
    if ((i==n-1)||(prevx < x[i+1])) {
      p = (IloFunctionSegmentI*)insertLast(x[i]);
      p->_slope = s[i+1];
      if (i<n-1)
	prevx = x[i+1];
    }
    ++i;
  }

  // Compute segment values using reference point, slopes and
  // discontinuities
  initCursor(x0);
  IloFunctionSegmentI* seg0 = getCursor();
  if (seg0->getXLeft() != -IloInfinity){
    seg0->setValueRef(y0 - (seg0->getSlope())*(x0 - seg0->getXLeft()));
  } else {
    seg0->setValueRef(y0 - (seg0->getSlope())*(x0 - seg0->getXRight()));
  }

  IloFunctionSegmentI* seg;
  for (operator++();ok(); operator++()){
    seg = (IloFunctionSegmentI*)getCursor()->getPrev();
    IloNum xseg = getCursor()->getXLeft();
    getCursor()->setValueRef(seg->getValueRight()+ IloGetDeltaSegmentedFunction(xseg, x, s));
  }
  
  if (seg0 != (IloFunctionSegmentI*)_header) {
    setCursor(seg0);
    for (operator--();ok(); operator--()){
      seg = (IloFunctionSegmentI*)getCursor()->getNext();
      IloNum xseg = seg->getXLeft();
      if (getCursor()->getXLeft() != -IloInfinity){
	getCursor()->setValueRef(seg->getValueLeft() -
				 (getCursor()->getSlope())*
				 (seg->getXLeft() - getCursor()->getXLeft()) -
				 IloGetDeltaSegmentedFunction(xseg, x, s));
      } else {
	getCursor()->setValueRef(seg->getValueLeft()-
				 IloGetDeltaSegmentedFunction(xseg, x, s));
	break;
      }
    }
  }
  setReferenceValue(x0,y0);
  _valuesComputed = IloTrue;
  notifyChange();
}

void IloSegmentedFunctionI::setPiecewiseLinear(IloNumArray x, 
                                               IloNumArray s,
                                               IloNum x0, 
                                               IloNum y0)
{
  assert (x.getSize()+1 == s.getSize());
  assert (x0 >= getXMin());
  assert (x0 < getXMax());

  _valuesComputed = IloFalse;

  // Clean the function
  reset();
  
  IloFunctionSegmentI* p = (IloFunctionSegmentI*)_header;
 
  if (x[0] != _xMin) {
    p->_slope = s[0];
  } else {
    p->_slope = s[1];
  }
  
  IloInt iSize = x.getSize();
  IloInt i =0;
  IloNum prevx = _xMin;
  while (i < iSize) {
    // skip bad data
    if ((x[i] > prevx) && (x[i]<_xMax)){
      p = (IloFunctionSegmentI*)insertLast(x[i]);
      p->_slope = s[i];
      prevx = x[i];
    }
    ++i;
  }
  
  setReferenceValue(x0,y0);
  notifyChange();
}

void IloSegmentedFunctionI::setSlope(IloNum x, IloNum s)
{
  assert (x >= getXMin());
  assert (x < getXMax());

  _valuesComputed = IloFalse;
  initCursor(x);
  if (getCursor()->getXLeft() == x){
    getCursor()->setSlope(s);
  } else {
    IloFunctionSegmentI* p = (IloFunctionSegmentI*)insertSlice0(x);
    p->_slope = s;
  }
  notifyChange();
}

void IloSegmentedFunctionI::needValues()
{
  if (_valuesComputed)
    return;
  
  initCursor(_xRefPoint);
  IloFunctionSegmentI* seg0 = getCursor();
  if (seg0->getXLeft() != -IloInfinity){
    seg0->setValueRef(_vRefPoint - 
		      (seg0->getSlope())*( _xRefPoint - seg0->getXLeft()));
  } else {
    seg0->setValueRef(_vRefPoint - 
		      (seg0->getSlope())*( _xRefPoint - seg0->getXRight()));
  }
  IloFunctionSegmentI* seg;
  for (operator++();ok(); operator++()){
    seg = (IloFunctionSegmentI*)getCursor()->getPrev();
    getCursor()->setValueRef(seg->getValueRight());
  }
  
  setCursor(seg0);
  for (operator--();ok(); operator--()){
    seg = (IloFunctionSegmentI*)getCursor()->getNext();
    if (getCursor()->getXLeft()!= -IloInfinity){
      getCursor()->setValueRef(seg->getValueLeft() -
			       (getCursor()->getSlope())*
			       (seg->getXLeft() - getCursor()->getXLeft()));
    } else {
      getCursor()->setValueRef(seg->getValueLeft());
    }
  }
  _valuesComputed = IloTrue;
}

void IloSegmentedFunctionI::setPoints(IloNum sl,
				      IloNumArray x, 
				      IloNumArray v,
				      IloNum sr)
{
  assert (x.getSize() == v.getSize());

  IloInt iSize = x.getSize() - 1;

  // Clean the function
  reset();
  
  IloFunctionSegmentI* p = (IloFunctionSegmentI*)_header;

  if ((0<iSize) && ((x[1] < x[0]) || 
		    ((x[1] == x[0]) && (v[1] == v[0]))))
    throw(Exception("Bad array to initialize a segment function"));
  
  p->setValueRef(v[0]);
  p->setSlope(sl);

  IloInt first = ((0<iSize) && (v[1] == v[0]))? (IloInt)1 : (IloInt)0;
  
  for (IloInt i=first; i < iSize; ++i) {
    if ((i > 0) && (x[i] < x[i-1]))
      throw(Exception(
		      "Bad array to initialize a segment function"));
    if ((i == 0) || (x[i] > x[i-1]))
      p = (IloFunctionSegmentI*)insertLast(x[i]);
    p->_valueRef = v[i];
    p->_slope = (v[i+1]-v[i])/(x[i+1]-x[i]);
  }
  if ((iSize == first) || (p->_slope != 0)) {
    if ((iSize==0)||(x[iSize] > x[iSize-1]))
      p = (IloFunctionSegmentI*)insertLast(x[iSize]);
    p->_valueRef = v[iSize];
    p->_slope = sr;
  }

  notifyChange();
}

void IloSegmentedFunctionI::setPoints(IloNumArray x, 
				      IloNumArray v)
{
  assert (x.getSize() == v.getSize());

  IloInt iSize = x.getSize() - 1;

  // Clean the function
  reset();
  
  IloFunctionSegmentI* p = (IloFunctionSegmentI*)_header;

  if ((x[1] < x[0]) || 
      ((x[1] == x[0]) && (v[1] == v[0])))
    throw(Exception(
		    "Bad array to initialize a segment function"));

  p->setValueRef(v[0]);
  p->setSlope(0.0);
  IloInt first = (v[1] == v[0])? (IloInt)1 : (IloInt)0;

  for (IloInt i=first; i < iSize; ++i) {
    if ((i > 0) && (x[i] < x[i-1]))
      throw(Exception(
		      "Bad array to initialize a segment function"));
    if ((i == 0) || (x[i] > x[i-1]))
      p = (IloFunctionSegmentI*)insertLast(x[i]);
    p->_valueRef = v[i];
    p->_slope = (v[i+1]-v[i])/(x[i+1]-x[i]);
  }
  if ((iSize == first) || (p->_slope != 0)) {
    if (x[iSize] > x[iSize-1])
      p = (IloFunctionSegmentI*)insertLast(x[iSize]);
    p->_valueRef = v[iSize];
    p->_slope = 0.0;
  }

  notifyChange();
}

IloNum IloSegmentedFunctionI::getValue(IloNum x)
{
  needValues();
  IloFunctionSegmentI* seg = (IloFunctionSegmentI*)seek(x);
  return seg->getValue(x);
}

IloNum IloSegmentedFunctionI::getValueLeft(IloNum x)
{
  assert (x != _xMin);
  needValues();
  IloFunctionSegmentI* seg = (IloFunctionSegmentI*)seek(x);
  if (seg->getXLeft() == x){
    return ((IloFunctionSegmentI*)(seg->getPrev()))->getValueRight();
  }
  return seg->getValue(x);
}

IloNum IloSegmentedFunctionI::getSlope(IloNum x)
{
  needValues();
  IloFunctionSegmentI* seg = (IloFunctionSegmentI*)seek(x);
  return seg->getSlope();
}

IloNum IloSegmentedFunctionI::getMax(IloNum x1, IloNum x2)
{
  if (x1 > x2) {
    // empty interval.  Try to make sense of it by
    // reversing the bounds.  Note if x1 == x2 the getValue(x1) is
    // returned. 
    IloNum tmp = x2;
    x2 = x1;
    x1 = tmp;
  }

  needValues();
  initCursor(x1);
  IloNum max = getCursor()->getValue(x1);
  if (x2 <= getCursor()->getXRight()){
    return IloMax(max, getCursor()->getValue(x2));
  } else {
    max = IloMax(max, getCursor()->getValueRight());

    for(operator++();
        ok() && (getCursor()->getXRight() < x2); 
        operator++()){
      max = IloMax(max, getCursor()->getMax());
    }
    
    if (ok()){
      max = IloMax(max, getCursor()->getValueLeft());
      max = IloMax(max, getCursor()->getValue(x2));
    }
  }
  return max;
} 

IloNum IloSegmentedFunctionI::getMin(IloNum x1, IloNum x2)
{
  if (x1 > x2) {
    // empty interval.  Try to make sense of it by
    // reversing the bounds.  Note if x1 == x2 the getValue(x1) is
    // returned. 
    IloNum tmp = x2;
    x2 = x1;
    x1 = tmp;
  }

  needValues();
  initCursor(x1);
  IloNum min = getCursor()->getValue(x1);
  if (x2 <= getCursor()->getXRight()){
    return IloMin(min, getCursor()->getValue(x2));
  } else {
    min = IloMin(min, getCursor()->getValueRight());

    for(operator++();
        ok() && (getCursor()->getXRight() < x2); 
        operator++()){
      min = IloMin(min, getCursor()->getMin());
    }
    
    if (ok()){
      min = IloMin(min, getCursor()->getValueLeft());
      min = IloMin(min, getCursor()->getValue(x2));
    }
  }
  return min;
}
  
IloNum IloSegmentedFunctionI::getArea(IloNum x1, IloNum x2)
{
  if (x1 > x2) {
    // empty interval.  Try to make sense of it by
    // reversing the bounds.  Note if x1 == x2, 0 is (correctly)
    // returned.
    IloNum tmp = x2;
    x2 = x1;
    x1 = tmp;
  }

  needValues();
  initCursor(x1);
  IloNum area = 0;
  if (x2 <= getCursor()->getXRight()){
    IloNum mean = (getCursor()->getValue(x1) + getCursor()->getValue(x2)) / 2.0;
    if (mean != 0.) {
      area += (x2-x1) * mean;
    }
    return area;
  } else {
    IloNum mean = (getCursor()->getValue(x1) + getCursor()->getValueRight()) / 2.0;
    if (mean != 0.) {
      area += (getCursor()->getXRight() - x1) * mean;
    }
    
    for(operator++();
        ok() && (getCursor()->getXRight() < x2); 
        operator++()){
      area += getCursor()->getArea();
    }
    
    if (ok()){
      IloNum mean2 = (getCursor()->getValue(x2) + getCursor()->getValueLeft()) / 2.0;
      if (mean2 != 0.) {
        area += (x2 - getCursor()->getXLeft()) * mean2;
      }
    }
  }
  return area;
}

void IloSegmentedFunctionI::doRestrictInterval(IloNum& x1, IloNum& x2,
                                               IloNum& v1, IloNum s) {
  if (x1 < _xMin) {
    if ((s != 0) && (x1 != - IloInfinity)) {
      // v1 associated with x1
      v1 += s * (_xMin - x1);
    }
    x1 = _xMin;
  }

  if (x2 > _xMax) {
    if ((s != 0) && (x1 == - IloInfinity)) {
      // v1 associated with x2
      v1 += s * (_xMax - x2);
    }
    x2 = _xMax;
  }
}

// For each x in [x1,x2), f(x) = v1 + s.(x-x1)
IloBool IloSegmentedFunctionI::doSetValue(IloNum x1, IloNum x2,  
                                          IloNum v1, IloNum s)
{
  doRestrictInterval(x1, x2, v1, s);
  if (x1 >= x2) return IloFalse;  // ignore empty interval

  needValues();

  
  if (x1 == - IloInfinity) {
    if (x2 == IloInfinity) {
      reset();
      IloFunctionSegmentI *seg = getBeginSegment();
      seg->setValueRef(v1);
      seg->setSlope(s);
      return IloTrue;
    }

    IloFunctionSegmentI *seg1 = (IloFunctionSegmentI*)searchSlice0(x1);
    assert(seg1 == _header);
    IloFunctionSegmentI *seg2 = (IloFunctionSegmentI*)searchSlice1(x2);
    assert(((seg1->getXRight() > x2) && (seg2->getPrev() == seg1))
           || (seg2->getXLeft() <= x2));

    if (seg1->getXRight() > x2) {
      IloFunctionSegmentI* seg = (IloFunctionSegmentI*)insertSlice0(x2);
      if (seg->getXRight() == IloInfinity) {
        assert(seg1->getSlope() == 0);
        seg->setValueRef(seg1->getValueRef());
        seg->setSlope(0.);
      }
      else {
        seg->setValueRef(seg1->getValueRef() + 
                         (x2 - seg->getXRight()) * seg1->getSlope());
        seg->setSlope(seg1->getSlope());
      }
    }
    else {
      if (seg2->isSameAs(x1, v1, s)) {
        nextSlice1();
      } else {
        seg2->addValueRef((x2 - seg2->getXLeft()) * seg2->getSlope());
        seg2->setStart( x2 );
      }
      remove01();
    }
    seg1->setValueRef(v1);
    seg1->setSlope(s);
    return IloTrue;
    
  }
  
  IloFunctionSegmentI *seg = getEndSegment();
  IloFunctionSegmentI *seg1, *seg2;

  // START OPTIM
  
  //    TODO: TREAT x1=seg->_start AND MERGING WITH seg->_prev. 
  if (x1 >= seg->getStart()) {
    if (seg->isSameAs(x1,v1,s))
      return IloFalse;
    else {
      IloNum start = seg->getStart();
      IloNum end   = seg->getXRight();;
      IloNum value = seg->getValueRef();
      IloNum slope = seg->getSlope();
      if (x1 > start) {
        seg1 = (IloFunctionSegmentI*)insertLast(x1);
        seg1->setValueRef(v1);
        seg1->setSlope(s);
      } else {
        if (seg->_prev && 
            ((IloFunctionSegmentI*)seg->_prev)->isSameAs(x1,v1,s)) {
          if (x2 >= _xMax)
            removeLast();
          else {
            seg->setStart( x2 );
            seg->setValueRef(value + (x2 - start) * slope);
          }
          return IloTrue;
        }
        seg->setValueRef(v1);
        seg->setSlope(s);
      }
      if (x2 < _xMax) {
        seg1 = (IloFunctionSegmentI*)insertLast(x2);
        if (slope == 0) 
          seg1->setValueRef(value);
        else if (start == - IloInfinity)
          seg1->setValueRef(value + (x2 - end) * slope);
        else
          seg1->setValueRef(value + (x2 - start) * slope);
        seg1->setSlope(slope);
      }
      return IloTrue;
    }
  }
 
  // END OPTIM

  seg1 = (IloFunctionSegmentI*)searchSlice0(x1);
  IloNum xRight = seg1->getXRight();
  if (seg1->isSameAs(x1, v1, s)) {
    if (xRight >= x2)
      return IloFalse;
    seg2 = (IloFunctionSegmentI*)searchSlice1(x2);
    if (seg2->isSameAs(x1, v1, s)) {
      nextSlice1();
    } else {
      seg2->addValueRef((x2 - seg2->getXLeft()) * seg2->getSlope());
      seg2->setStart(x2);
    }
    if (seg1->getStart() == x1 && seg1->getStart() > - IloInfinity)
      nextSlice0();
    remove01();
    return IloTrue;
  } else if ((seg1->getStart() == x1) &&
             (((IloFunctionSegmentI*)(seg1->getPrev()))->isSameAs(x1, v1, s))) { 
    if (xRight > x2) {
      seg1->addValueRef((x2 - seg1->getXLeft()) * seg1->getSlope());
      seg1->setStart( x2 );
      return IloTrue;
    }
    seg2 = (IloFunctionSegmentI*)searchSlice1(x2);
    if (seg2->isSameAs(x1, v1, s)) {
      nextSlice1();
    } else {
      seg2->addValueRef((x2 - seg2->getXLeft()) * seg2->getSlope());
      seg2->setStart( x2 );
    }
    remove01();
    return IloTrue;
  } else {
    seg = (IloFunctionSegmentI*)seg1->getNext();
    
    if ((x2 >= seg->getXLeft()) && 
        (x2 <= seg->getXRight()) && 
        (seg->isSameAs(x1, v1, s))) {
      seg->addValueRef((x1 - seg->getXLeft()) * seg->getSlope());
      seg->setStart( x1 );
      if (seg1->getStart() == x1)
        removeSlice0R();
      return IloTrue;
    }

    if (xRight >= x2){    
      if (seg1->getStart()== x1)
        nextSlice0();
      if (xRight > x2) {
        seg = (IloFunctionSegmentI*)insertSlice0(x2);
        seg->_valueRef = seg1->getValue(x2);
        seg->_slope = seg1->getSlope();
      }
      if (seg1->getStart() == x1) {
        seg1->_valueRef = v1;
        seg1->_slope = s;
      } else {
        seg = (IloFunctionSegmentI*)insertSlice0(x1);
        seg->_valueRef = v1;
        seg->_slope = s;
      }
      return IloTrue;
    }

    seg2 = (IloFunctionSegmentI*)searchSlice1(x2);
    if (seg2->isSameAs(x1, v1, s)) {
      seg2->addValueRef((x1 - seg2->getXLeft()) * seg2->getSlope());
      seg2->setStart( x1 );
    } else {
      seg = (IloFunctionSegmentI*)insertSlice0(x1);
      seg->_valueRef = v1;
      seg->_slope = s;
      if (x2 < seg2->getXRight()) {
        seg2->addValueRef((x2 - seg2->getXLeft()) * seg2->getSlope());
        seg2->setStart( x2 );
      }      
      nextSlice0();
    }
    remove01();
    return IloTrue;
  }
}

IloBool IloSegmentedFunctionI::doAddValue(IloNum x1, IloNum x2, IloNum v1, IloNum s) {
  doRestrictInterval(x1, x2, v1, s);
  if (x1 >= x2) return IloFalse;  // ignore empty interval

  if ((v1 == 0) && (s == 0))
    return IloFalse;

  needValues();

  IloFunctionSegmentI *seg1 = (IloFunctionSegmentI*)searchSlice0(x1);
  IloFunctionSegmentI *seg;
  if (seg1->getStart()== x1) {
    IloFunctionSegmentI* prev = (IloFunctionSegmentI*)seg1->_prev;
    if ((prev != getBeginSegment()) &&
	(prev->isSameAs(x1, seg1->_valueRef + v1, seg1->_slope + s))) {
      IloNum x = prev->getStart();
      searchSlice0(x);
      removeSlice0R();
      seg1->setStart(x);
      if (seg1->_slope != 0)
	seg1->setValueRef(seg1->_valueRef + (x-x1)*seg1->_slope);
    }
  } else {
    seg = seg1;
    seg1 = (IloFunctionSegmentI*)insertSlice0(x1);
    seg1->_valueRef = seg->getValue(x1);
    seg1->_slope    = seg->_slope;
  }

  while (seg1->getXRight() < x2) {
    if (s == 0.)
      seg1->addValueRef(v1);
    else {
      seg1->addValueRef(v1 + s * (seg1->getXLeft() - x1));
      seg1->addSlope(s);
    }
    seg1 = (IloFunctionSegmentI*)seg1->getNext();
  }
  
  if (seg1->getXRight() == x2) {
    if (s == 0.)
      seg1->addValueRef(v1);
    else {
      seg1->addValueRef(v1 + s * (seg1->getXLeft() - x1));
      seg1->addSlope(s);
    }
    if (((IloFunctionSegmentI*)seg1->getNext())->isSameAs(seg1->getStart(), seg1->_valueRef, seg1->_slope)) {
      searchSlice0(x2);
      removeSlice0R();
    }
    return IloTrue;
  } else {
    seg = seg1;
    searchSlice0(x2);
    seg1 = (IloFunctionSegmentI*)insertSlice0(x2);
    seg1->_valueRef = seg->getValue(x2);
    seg1->_slope    = seg->_slope;
    if (s == 0.)
      seg->addValueRef(v1);
    else {
      seg->addValueRef(v1 + s * (seg->getXLeft() - x1));
      seg->addSlope(s);
    }
  }
  return IloTrue;
}
  
IloBool IloSegmentedFunctionI::doSetMax(IloNum x1, IloNum x2, 
					IloNum v1, IloNum s)
{
  doRestrictInterval(x1, x2, v1, s);
  if (x1 >= x2) return IloFalse;  // ignore empty interval
  
  needValues();
  
  // fast test to do nothing
  IloNum max;
  if (s==0) 
    max = v1;
  else if (x1==-IloInfinity) 
    max = (s<0) ? IloInfinity : v1;
  else
    max = (s<0) ? v1 : v1 + (x2-x1)*s;
  IloFunctionSegmentI* p = (IloFunctionSegmentI*) searchSlice0(x1);
  if ((p->getXLeft() <= x1) && (p->getXRight() >= x2) &&
      (p->getMin() >= max))
    return IloFalse;

  _intervals->reset();
  IloBool changed = IloFalse;
  IloNum ax2, ax1;
  while (p->getXLeft() < x2) { 
    if (p->maxInterval(x1, x2, v1, s, ax1, ax2) && ax1 + EPSILON < ax2) { // added && ax1 + EPSILON < ax2
      changed = IloTrue;
      _intervals->addInterval(ax1, ax2, 0);
    }      
    p = (IloFunctionSegmentI*) p->getNext();
  }
  
  IloIntervalI* it = (IloIntervalI*)_intervals->getHeader()->getNext();
  while (it->getStart() < IloInfinity) {
    IloNum start = it->getStart();
    IloNum end = it->getEnd();
    IloNum v = v1;
    if (0 != s) {
      if (x1 == -IloInfinity) {
	v += (((start == -IloInfinity)?end:start) - x2) * s;
      }
      else {
	assert(start != -IloInfinity);
	v += (start - x1) * s;
      }
    }
    doSetValue(start, end, v, s);
    it = (IloIntervalI*) it->getNext();
  }
  
  return changed;
}

IloBool IloSegmentedFunctionI::doSetMin(IloNum x1, IloNum x2, 
					IloNum v1, IloNum s)
{
  doRestrictInterval(x1, x2, v1, s);
  if (x1 >= x2) return IloFalse;  // ignore empty interval
  
  needValues();
  
  // fast test to do nothing
  IloNum min;
  if (s==0) 
    min = v1;
  else if (x1==-IloInfinity) 
    min = (s>0) ? -IloInfinity : v1;
  else
    min = (s>0) ? v1 : v1 + (x2-x1)*s;
  IloFunctionSegmentI* p = (IloFunctionSegmentI*) searchSlice0(x1);
  if ((p->getXLeft() <= x1) && (p->getXRight() >= x2) &&
      (p->getMax() <= min))
    return IloFalse;

  _intervals->reset();
  IloBool changed = IloFalse;
  IloNum ax2, ax1;
  while (p->getXLeft() < x2) { 
    if (p->minInterval(x1, x2, v1, s, ax1, ax2) && ax1 + EPSILON < ax2) { // added && ax1 + EPSILON < ax2 
      changed = IloTrue;
      _intervals->addInterval(ax1, ax2, 0);
    }      
    p = (IloFunctionSegmentI*) p->getNext();
  }
  
  IloIntervalI* it = (IloIntervalI*)_intervals->getHeader()->getNext();
  while (it->getStart() < IloInfinity) {
    IloNum start = it->getStart();
    IloNum end = it->getEnd();
    IloNum v = v1;
    if (0 != s) {
      if (x1 == -IloInfinity) {
	v += (((start == -IloInfinity)?end:start) - x2) * s;
      } else {
	assert(start != -IloInfinity);
	v += (start - x1) * s;
      }
    }
    doSetValue(start, end, v, s);
    it = (IloIntervalI*) it->getNext();
  }

  return changed;
}

IloBool IloSegmentedFunctionI::doShift(IloNum dx, IloNum dval) {
  if (dx == 0) return IloFalse;
  needValues();
  if (dx > 0) {
    IloNum xMax = _xMax - dx;
    IloFunctionSegmentI* p = getEndSegment();
    doSetValue(xMax, _xMax, p->getValueRef(), p->getSlope());
    p = (IloFunctionSegmentI*)getHeader()->getNext(); 
    while (p->getStart() < IloInfinity) {
      p->setStart( p->getStart() + dx );
      p = (IloFunctionSegmentI*)p->getNext(); 
    }
    doSetValue(_xMin, _xMin+dx, dval, 0);
    return IloTrue;
  } else if (dx < 0) {
    IloNum xMin = _xMin - dx;
    IloFunctionSegmentI* p = getBeginSegment();
    if (-IloInfinity < _xMin) 
      doSetValue(_xMin, xMin, p->getValueRef(), p->getSlope());
    p = (IloFunctionSegmentI*)getHeader()->getNext(); 
    while (p->getStart() < IloInfinity) {
      p->setStart( p->getStart() + dx );
      p = (IloFunctionSegmentI*)p->getNext(); 
    }
    if (_xMax < IloInfinity) 
      doSetValue(_xMax+dx, _xMax, dval, 0);
    return IloTrue;
  }
  return IloTrue;
}

IloBool
IloSegmentedFunctionI::setValue(IloNum x1, IloNum x2,  IloNum v1, IloNum s) {
  if (doSetValue(x1, x2, v1, s)) {
    notifyChange();
    return IloTrue;
  }
  return IloFalse;
}

IloBool
IloSegmentedFunctionI::setValue(IloNum x1, IloNum x2, IloSegmentedFunctionI* fct) {
  if (fct == this) {
    // THROW ERROR	
  }
  needValues();
  fct->needValues();
  IloNum fmin = fct->getXMin();
  IloNum fmax = fct->getXMax();
  IloNum t = x1;
  while (t < x2) {
    for (fct->initCursor(fct->getXMin());
	 fct->ok();
	 fct->operator++()){
      IloFunctionSegmentI* s = fct->getCursor();
      IloNum smin = IloMax(fmin,s->getXLeft());
      IloNum smax = IloMin(fmax,s->getXRight());
      doSetValue(t, IloMin(x2,t+(smax-smin)),
		 s->getValueRef(), s->getSlope());
      t+=(smax-smin);
      if (x2 <= t)
	break;
    }
  }
  notifyChange();
  return IloTrue;
}

IloBool
IloSegmentedFunctionI::addValue(IloNum x1, IloNum x2, IloNum v1, IloNum s) {
  if (doAddValue(x1, x2, v1, s)) {
    notifyChange();  
    return IloTrue;
  }
  return IloFalse;
}
  
IloBool
IloSegmentedFunctionI::setMax(IloNum x1, IloNum x2, IloNum v1, IloNum s) {
  ensureIntervals();
  if (doSetMax(x1, x2, v1, s)) {
    notifyChange();  
    return IloTrue;
  }
  return IloFalse;
}

IloBool
IloSegmentedFunctionI::setMin(IloNum x1, IloNum x2, IloNum v1, IloNum s) {
  ensureIntervals();
  if (doSetMin(x1, x2, v1, s)) {
    notifyChange();  
    return IloTrue;
  }
  return IloFalse;
}

IloBool
IloSegmentedFunctionI::shift(IloNum dx, IloNum dval) {
  if (doShift(dx, dval)) {
    notifyChange();     
    return IloTrue;
  }
  return IloFalse;
}

IloBool IloSegmentedFunctionI::setPeriodic(IloSegmentedFunctionI* f, 
                                           IloNum x0, 
                                           IloNum n, IloNum dval)
{
  assert (x0 >= _xMin);
  assert (x0 <  _xMax);
  assert (f->getXMax() < +IloInfinity);
  assert (f->getXMin() > -IloInfinity);
  assert (n > 0);

  reset();

  IloNum fxmin = f->getXMin();
  IloNum fxmax = f->getXMax();
  IloNum fspan = fxmax - fxmin;
  IloNum x1 = IloMin(x0 + n*fspan, _xMax);
  IloBool stopFlag = IloFalse;
  IloFunctionSegmentI* p;
  IloNum newXLeft, dur;
  doSetValue(_xMin, x0, dval, 0);
  for (IloInt i=0; ((i<n) && (!stopFlag)) ; i++){
    p = (IloFunctionSegmentI*)f->getHeader()->getNext(); 
    while (p->getStart() < fxmax && !stopFlag) {
      newXLeft = x0 + (i*fspan) + p->getXLeft() - fxmin;
      dur = IloMin(fxmax,p->getXRight()) - p->getXLeft();
      if (newXLeft < _xMax){
        doSetValue(newXLeft, newXLeft + dur, p->getValueLeft(), p->getSlope());
      } else {
        stopFlag = IloTrue;
      }
      p = (IloFunctionSegmentI*)p->getNext(); 
    }
  }

  if (x1 < _xMax)
    doSetValue(x1, _xMax, dval, 0);

  return IloTrue;
}

IloBool
IloSegmentedFunctionI::setAdd(IloSegmentedFunctionI* fct)
{
  if (fct == this)
    return setMult(2.0);
  IloBool changed = IloTrue;
  needValues();
  fct->needValues();
  for (fct->initCursor(fct->getXMin());
       fct->ok();
       fct->operator++()){
    IloFunctionSegmentI* seg = fct->getCursor();	
    changed |= doAddValue(seg->getXLeft(), seg->getXRight(),
			  seg->getValueRef(), seg->getSlope());
  }
  if (changed)
    notifyChange();
  return changed;
}

IloBool
IloSegmentedFunctionI::setSub(IloSegmentedFunctionI* fct)
{
  if (fct == this)
    return setValue(_xMin, _xMax, 0, 0); 
  IloBool changed = IloTrue;
  needValues();
  fct->needValues();
  for (fct->initCursor(fct->getXMin());
       fct->ok();
       fct->operator++()){
    IloFunctionSegmentI* seg = fct->getCursor();
    changed |= doAddValue(seg->getXLeft(), seg->getXRight(),
                          -seg->getValueRef(), -seg->getSlope());
  }
  if (changed)
    notifyChange();
  return changed;
}

IloBool IloSegmentedFunctionI::isSameAs(IloSegmentedFunctionI* fct)
{
  if (fct == this)
    return IloTrue;

  if ((fct->getXMin() != getXMin()) ||
      (fct->getXMax() != getXMax())){
    return IloFalse;
  }
  
  needValues();
  fct->needValues();
  fct->initCursor(fct->getXMin());
  while (fct->getSegmentMin() < _xMax){
    IloFunctionSegmentI* fseg = fct->getCursor();
    IloNum fxLeft = IloMax(fseg->getXLeft(), _xMin);
    if ((getValue(fxLeft) != fseg->getValueLeft()) ||
        (getSlope(fxLeft) != fseg->getSlope())) {
      return IloFalse;
    }
    fct->operator++();
  }
  
  initCursor(getXMin());
  while (getSegmentMin() < _xMax){
    IloFunctionSegmentI* seg  = getCursor();
    IloNum xLeft = IloMax(seg->getXLeft(), _xMin);
    if ((fct->getValue(xLeft) != seg->getValueLeft()) ||
        (fct->getSlope(xLeft) != seg->getSlope())) {
      return IloFalse;
    }
    operator++();
  }
  
  return IloTrue;
}

IloBool IloSegmentedFunctionI::isLowerThan(IloSegmentedFunctionI* fct)
{
  if ((fct->getXMin() != getXMin()) ||
      (fct->getXMax() != getXMax())){
    return IloFalse;
  }

  needValues();
  fct->needValues();
  fct->initCursor(fct->getXMin());
  while (fct->getSegmentMin() < _xMax){
    IloFunctionSegmentI* fseg = fct->getCursor();
    IloNum fxLeft  = IloMax(fseg->getXLeft(),  _xMin);
    IloNum fxRight = IloMin(fseg->getXRight(), _xMax);
    if ((getValue(fxLeft) > fseg->getValueLeft()) ||
        (getValueLeft(fxRight) > fseg->getValueRight()))
      return IloFalse;
    fct->operator++();
  }
  
  initCursor(getXMin());
  while (getSegmentMin() < _xMax){
    IloFunctionSegmentI* seg  = getCursor();
    IloNum xLeft  = IloMax(seg->getXLeft(),  _xMin);
    IloNum xRight = IloMin(seg->getXRight(), _xMax);
    if ((fct->getValue(xLeft) < seg->getValueLeft()) ||
        (fct->getValueLeft(xRight) < seg->getValueRight()))
      return IloFalse;
    operator++();
  }

  return IloTrue;
}

IloBool
IloSegmentedFunctionI::setMult(IloNum k)
{
  if (k == 1)
    return IloFalse;
  if (k == 0)
    return setValue(_xMin, _xMax, 0, 0);
  for (initCursor(getXMin());
       ok();
       operator++()){
    IloFunctionSegmentI* seg = getCursor();
    seg->setValueRef(seg->getValueRef() * k);
    seg->setSlope(seg->getSlope() * k);
  }
  notifyChange();
  return IloTrue;
}

IloBool IloSegmentedFunctionI::setXScale(IloNum k)
{
  assert(k != 0);

  if (k == 1)
    return IloFalse;

  if (k > 1) {
    if (_xMin > (-IloInfinity / k))
      _xMin *= k;
    else
      _xMin = -IloInfinity;
    
    if (_xMax < (IloInfinity / k))
      _xMax *= k;
    else
      _xMax = IloInfinity;
  } else {
    _xMin *= k;
    _xMax *= k;
  }

  IloFunctionSegmentI* p = (IloFunctionSegmentI*)_header->getNext();

  while (!p->isTail()) {
    p->setStart( p->getStart() * k );
    p->setSlope( p->getSlope() * k );
    p = (IloFunctionSegmentI*) p->getNext();
  }
  
  notifyChange();
  return IloTrue;
}

IloBool
IloSegmentedFunctionI::setMin(IloSegmentedFunctionI* fct)
{
  if (fct == this)
    return IloFalse;
  IloBool changed = IloFalse;
  ensureIntervals();
  for (fct->initCursor(fct->getXMin());
       fct->ok();
       fct->operator++()){
    IloFunctionSegmentI* seg = fct->getCursor();
    changed |= doSetMin(seg->getXLeft(), seg->getXRight(),
                        seg->getValueRef(), seg->getSlope());
  }
  if (changed)
    notifyChange();
  return changed;
}

IloBool
IloSegmentedFunctionI::setMax(IloSegmentedFunctionI* fct)
{
  if (fct == this)
    return IloFalse;
  IloBool changed = IloFalse;
  ensureIntervals();
  for (fct->initCursor(fct->getXMin());
       fct->ok();
       fct->operator++()){
    IloFunctionSegmentI* seg = fct->getCursor();
    changed |= doSetMax(seg->getXLeft(), seg->getXRight(),
                        seg->getValueRef(), seg->getSlope());
  }
  if (changed)
    notifyChange();
  return changed;
}

IloBool
IloSegmentedFunctionI::setPeriodicValue(IloNum x1, IloNum x2,
					IloSegmentedFunctionI* fct,
					IloNum offset) {
  if (fct == this) {
    // THROW ERROR	
  }
  needValues();
  fct->needValues();
  IloNum fmin = fct->getXMin();
  IloNum fmax = fct->getXMax();
  IloNum delta = offset;
  if (delta > (fmax-fmin)) {
    IloNum n = IloFloor(delta/(fmax-fmin));
    delta -= n*(fmax-fmin);
  }
	
  IloNum t = x1;

  // In case first cell of fct is cut
  IloNum fstart;
  fct->initCursor(fmin + delta);
  IloFunctionSegmentI* s = fct->getCursor();
  if (s->getXLeft() < fmin + delta) {
    fstart = IloMin(fmax,s->getXRight());
    if (fstart == fmax)
      fstart = fmin;
    IloNum smin = fmin+delta;
    IloNum smax = IloMin(fmax,s->getXRight());
    doSetValue(t, IloMin(x2,t+(smax-smin)),
	       s->getValue(fmin + delta), s->getSlope());
    t+=(smax-smin);
  } else {
    fstart = fmin + delta;
  }
	
  while (t < x2) {
    for (fct->initCursor(fstart);
	 fct->ok();
	 fct->operator++()){
      IloFunctionSegmentI* s2 = fct->getCursor();
      IloNum smin = IloMax(fmin,s2->getXLeft());
      IloNum smax = IloMin(fmax,s2->getXRight());
      doSetValue(t, IloMin(x2,t+(smax-smin)),
		 s2->getValueRef(), s2->getSlope());
      t+=(smax-smin);
      if (x2 <= t)
	break;
    }
    fstart = fmin;
  }
	
  notifyChange();
  return IloTrue;
}

IloNum IloSegmentedFunctionI::getSegmentMin() const {
  assert(getCursor() != 0);
  return getCursor()->getXLeft();
}

IloNum IloSegmentedFunctionI::getSegmentMax() const {
  assert(getCursor() != 0);
  assert(getCursor()->getNext() != 0);
  return getCursor()->getXRight();
}

IloNum IloSegmentedFunctionI::getValue()      const {
  assert(getCursor() != 0);
  return getCursor()->getValueLeft();
}

// Called in case (dis/)allocation is needed
IloSkipListNodeI* IloSegmentedFunctionI::allocateNode(NodeType type, IloInt level)
{
  IloSkipListNodeI* node = new (_env) IloFunctionSegmentI(_env, level);
  if (type==headNode) {
    node->setStart( -IloInfinity );
  }
  else if (type==tailNode) {
    node->setStart( +IloInfinity );
  }
  else {}
  return node;
}
void IloSegmentedFunctionI::destroyNode(IloSkipListNodeI* node)
{
  IloFunctionSegmentI* seg = (IloFunctionSegmentI*)node;
  seg->~IloFunctionSegmentI();
  _env->free(node, sizeof(IloFunctionSegmentI));
}


void IloSegmentedFunctionI::display(ILOSTD(ostream)& out) const
{
  const char* name = getName();
  if (name)
    out << name << " ";
  else 
    out << "IloNumToNumSegmentedFunctionI(" << (IloAny) this << ") ";

  out << "<" << getXMin() << "> ";

  IloFunctionSegmentI* seg = getBeginSegment();
  if (!seg->isTail()) {
    seg->display(out);
    seg  = (IloFunctionSegmentI*) seg->getNext();
  }
  for(; !seg->isTail(); seg  = (IloFunctionSegmentI*) seg->getNext()) {
    out << ",";
    seg->display(out);
  }
  out << " <" << getXMax() << ">";
}

//-------------------------------------------------------
// class IloNumToNumStepFunction
//-------------------------------------------------------

IloNumToNumStepFunction::IloNumToNumStepFunction(const IloEnv env,
                                                 IloNum xmin, 
                                                 IloNum xmax,
                                                 IloNum dval,
                                                 const char* name)
  : IloParameter(new (env) 
		 IloSegmentedFunctionI(env.getImpl(), xmin, xmax, dval, name))
{}

IloNumToNumStepFunction::IloNumToNumStepFunction(const IloEnv env,
                                                 const IloNumArray x, 
                                                 const IloNumArray v,
                                                 IloNum xmin, 
                                                 IloNum xmax,
                                                 const char* name)
  : IloParameter(new (env) 
		 IloSegmentedFunctionI(env.getImpl(), xmin, xmax, 0., name))
{
  if (x.getSize()+1 != v.getSize())
    throw(IloParameterI::Exception(
				   "Bad array sizes for step function"));
  getImpl()->setSteps(x,v);
}

IloNumToNumStepFunction::IloNumToNumStepFunction(const IloEnv env, 
						 IloNum yl, 
						 const IloNumArray x, 
						 const IloNumArray y, 
						 const char* name) 
  : IloParameter(new (env) 
		 IloSegmentedFunctionI(env.getImpl(), -IloInfinity, IloInfinity, yl, name))
{
  if (x.getSize() != y.getSize())
    throw(IloParameterI::Exception("Bad array sizes for step function"));
  getImpl()->setSteps(yl,x,y);
}

// Returns the value of the invoking function at x. 
IloNum IloNumToNumStepFunction::getValue(IloNum x) const {
  assert(getImpl() != 0);
  if ((x < getDefinitionIntervalMin()) || (x >= getDefinitionIntervalMax()))
    throw(IloParameterI::Exception(
				   "Bad argument for getValue on step function"));
  return getImpl()->getValue(x);
}

// Returns the maximal value of the invoking function on the
// interval [x1,x2).
IloNum IloNumToNumStepFunction::getMax(IloNum x1, IloNum x2) const {
  assert(getImpl() != 0);
  if ((x1 < getDefinitionIntervalMin()) || (x2 > getDefinitionIntervalMax()))
    throw(IloParameterI::Exception(
				   "Bad argument for getMax on step function"));
  return getImpl()->getMax(x1, x2);
}

// Returns the minimal value of the invoking function on the
// interval [x1,x2).
IloNum IloNumToNumStepFunction::getMin(IloNum x1, IloNum x2) const {
  assert(getImpl() != 0);
  if ((x1 < getDefinitionIntervalMin()) || (x2 > getDefinitionIntervalMax()))
    throw(IloParameterI::Exception(
				   "Bad argument for getMin on step function"));
  return getImpl()->getMin(x1, x2);
}

// Return the integral of the invoking function on the interval
// [x1,x2).
IloNum IloNumToNumStepFunction::getArea(IloNum x1, IloNum x2) const{
  assert(getImpl() != 0);
  if ((x1 < getDefinitionIntervalMin()) || (x2 > getDefinitionIntervalMax()))
    throw(IloParameterI::Exception(
				   "Bad argument for getArea on step function"));
  return getImpl()->getArea(x1, x2);
}
 
ILOSTD(ostream)& 
operator<<(ILOSTD(ostream)& out, const IloNumToNumStepFunction& f) {
  IloInternalAssert(f.getImpl(), "operator<< with IloNumToNumStepFunction using empty handle.");
  f.getImpl()->display(out);
  return out;
}

IloNumToNumStepFunction operator*(const IloNumToNumStepFunction fct1,
                                  IloNum k) {
  assert( fct1.getImpl()!=0 );
  IloNumToNumStepFunctionI* fct = 
    (IloNumToNumStepFunctionI*) (fct1.getImpl()->makeCopy());
  fct->setMult(k);
  return (IloNumToNumStepFunction)fct;
}

IloNumToNumStepFunction operator*(IloNum k,
                                  const IloNumToNumStepFunction fct1){
  assert(fct1.getImpl()!=0 );
  return (fct1 * k);
}

IloNumToNumStepFunction operator+(const IloNumToNumStepFunction fct1,
                                  const IloNumToNumStepFunction fct2){
  IloNumToNumStepFunctionI* fct = 
    (IloNumToNumStepFunctionI*) (fct1.getImpl()->makeCopy());
  fct->setAdd(fct2.getImpl());
  return (IloNumToNumStepFunction)fct;
}

IloNumToNumStepFunction operator-(const IloNumToNumStepFunction fct1,
                                  const IloNumToNumStepFunction fct2){
  IloNumToNumStepFunctionI* fct = (IloNumToNumStepFunctionI*)(fct1.getImpl()->makeCopy());
  fct->setSub(fct2.getImpl());
  return (IloNumToNumStepFunction)fct;
}

IloNumToNumStepFunction IloMin(const IloNumToNumStepFunction fct1,
                               const IloNumToNumStepFunction fct2){
  IloNumToNumStepFunctionI* fct = (IloNumToNumStepFunctionI*)(fct1.getImpl()->makeCopy());
  fct->setMin(fct2.getImpl());
  return (IloNumToNumStepFunction)fct;
}

IloNumToNumStepFunction IloMax(const IloNumToNumStepFunction fct1,
                               const IloNumToNumStepFunction fct2){
  IloNumToNumStepFunctionI* fct = (IloNumToNumStepFunctionI*)(fct1.getImpl()->makeCopy());
  fct->setMax(fct2.getImpl());
  return (IloNumToNumStepFunction)fct;
}

//-------------------------------------------------------
// class IloNumToNumStepFunctionCursor
//-------------------------------------------------------

IloNumToNumStepFunctionCursor::
IloNumToNumStepFunctionCursor(const IloNumToNumStepFunction fct)
  : _function (fct.getImpl())
{
  assert(_function != 0);
  IloNum x = _function->getXMin();
  _cursor = (IloFunctionSegmentI*)(_function->seek(x));
  assert(_cursor !=0);
}

IloNumToNumStepFunctionCursor::
IloNumToNumStepFunctionCursor(const IloNumToNumStepFunction fct, IloNum x)
  : _function (fct.getImpl())
{
  assert(_function != 0);
  if ((x < _function->getXMin()) ||
      (x >= _function->getXMax()))
    throw(IloParameterI::Exception(
				   "Cannot create cursor out of function definition interval"));

  _cursor = (IloFunctionSegmentI*)(_function->seek(x));
  assert(_cursor !=0);
}

IloNumToNumStepFunctionCursor::
IloNumToNumStepFunctionCursor(const IloNumToNumStepFunctionCursor& curs)
  : _function(curs._function),
    _cursor(curs._cursor)
{}  

IloNumToNumStepFunctionCursor::
IloNumToNumStepFunctionCursor()
  : _function(0),
    _cursor(0)
{}  

IloNumToNumStepFunctionCursor::~IloNumToNumStepFunctionCursor() {}

void 
IloNumToNumStepFunctionCursor::operator=(const IloNumToNumStepFunctionCursor& csor) {
  _function = csor._function;
  _cursor   = csor._cursor;
}

void IloNumToNumStepFunctionCursor::seek(IloNum x) {
  if ((x < _function->getXMin()) ||
      (x >= _function->getXMax()))
    throw(IloParameterI::Exception(
				   "Cannot create cursor outside of function definition interval"));
  _cursor = (IloFunctionSegmentI*)(_function->seek(x));
  assert(_cursor !=0);  
}

//-------------------------------------------------------
// class IloNumToNumSegmentFunction
//-------------------------------------------------------


IloNumToNumSegmentFunction::IloNumToNumSegmentFunction(const IloEnv env,
                                                       IloNum xmin, 
                                                       IloNum xmax,
                                                       IloNum dval,
                                                       const char* name)
  : IloParameter(new (env) 
		 IloSegmentedFunctionI(env.getImpl(), xmin, xmax, dval, name))
{}


IloNumToNumSegmentFunction::
IloNumToNumSegmentFunction(const IloNumToNumStepFunction& numFunction)
  : IloParameter(numFunction.makeCopy().getImpl())
{}
 

IloNumToNumSegmentFunction::IloNumToNumSegmentFunction(const IloEnv env,
                                                       const IloNumArray x, 
                                                       const IloNumArray v,
                                                       IloNum xmin, 
                                                       IloNum xmax,
                                                       const char* name)
  : IloParameter(new (env) 
		 IloSegmentedFunctionI(env.getImpl(), xmin, xmax, 0., name))
{
  if ((x.getSize() != v.getSize()) || (x.getSize() < 2))
    throw(IloParameterI::Exception(
				   "Bad array sizes for segment function"));
  getImpl()->setPoints(x,v);
}
    
// TODO: MAKE IT A CONSTRUCTOR
IloNumToNumSegmentFunction IloPiecewiseLinearFunction(const IloEnv env, 
						      const IloNumArray points,
						      const IloNumArray slopes,
						      IloNum x0,
						      IloNum v0,
						      const char* name) {
  
  if ((points.getSize()+1 != slopes.getSize()) || (points.getSize() < 1))
    throw(IloParameterI::Exception("Bad array sizes for segment function"));
  IloSegmentedFunctionI* f = 
    new (env) IloSegmentedFunctionI(env.getImpl(), -IloInfinity, IloInfinity, 0., name);
  f->definePiecewiseLinear(points, slopes, x0, v0);
  return f;
}

IloNumToNumSegmentFunction::IloNumToNumSegmentFunction(const IloEnv env, 
						       IloNum sl, 
						       const IloNumArray x, 
						       const IloNumArray y, 
						       IloNum sr, 
						       const char* name)
  : IloParameter(new (env) 
		 IloSegmentedFunctionI(env.getImpl(), -IloInfinity, IloInfinity, 0., name))
{
  if ((x.getSize() != y.getSize()) || (x.getSize() < 1))
    throw(IloParameterI::Exception("Bad array sizes for segment function"));
  getImpl()->setPoints(sl,x,y,sr);
}

// Returns the value of the invoking function at x. 
IloNum IloNumToNumSegmentFunction::getValue(IloNum x) const {
  assert(getImpl() != 0);
  if ((x < getDefinitionIntervalMin()) || (x >= getDefinitionIntervalMax()))
    throw(IloParameterI::Exception(
				   "Bad argument for getValue on segment function"));
  return getImpl()->getValue(x);
}

// Returns the maximal value of the invoking function on the
// interval [x1,x2).
IloNum IloNumToNumSegmentFunction::getMax(IloNum x1, IloNum x2) const {
  assert(getImpl() != 0);
  if ((x1 < getDefinitionIntervalMin()) || (x2 > getDefinitionIntervalMax()))
    throw(IloParameterI::Exception(
				   "Bad argument for getMax on segment function"));
  return getImpl()->getMax(x1, x2);
}

// Returns the minimal value of the invoking function on the
// interval [x1,x2).
IloNum IloNumToNumSegmentFunction::getMin(IloNum x1, IloNum x2) const {
  assert(getImpl() != 0);
  if ((x1 < getDefinitionIntervalMin()) || (x2 > getDefinitionIntervalMax()))
    throw(IloParameterI::Exception(
				   "Bad argument for getMin on segment function"));
  return getImpl()->getMin(x1, x2);
}

// Returns the integral of the invoking function on the interval
// [x1,x2).
IloNum IloNumToNumSegmentFunction::getArea(IloNum x1, IloNum x2) const{
  assert(getImpl() != 0);
  if ((x1 < getDefinitionIntervalMin()) || (x2 > getDefinitionIntervalMax()))
    throw(IloParameterI::Exception(
				   "Bad argument for getArea on segment function"));
  return getImpl()->getArea(x1, x2);
}
 
ILOSTD(ostream)& 
operator<<(ILOSTD(ostream)& out, const IloNumToNumSegmentFunction& f) {
  IloInternalAssert(f.getImpl(), "operator<< with IloNumToNumSegmentFunction using empty	 handle.");
  f.getImpl()->display(out);
  return out;
}

IloNumToNumSegmentFunction operator*(const IloNumToNumSegmentFunction fct1,
                                     IloNum k) {
  assert( fct1.getImpl()!=0 );
  IloNumToNumSegmentFunctionI* fct = 
    (IloNumToNumSegmentFunctionI*) (fct1.getImpl()->makeCopy());
  fct->setMult(k);
  return (IloNumToNumSegmentFunction)fct;
}

IloNumToNumSegmentFunction operator*(IloNum k,
                                     const IloNumToNumSegmentFunction fct1){
  assert(fct1.getImpl()!=0 );
  return (fct1 * k);
}

IloNumToNumSegmentFunction operator+(const IloNumToNumSegmentFunction fct1,
                                     const IloNumToNumSegmentFunction fct2){
  IloNumToNumSegmentFunctionI* fct = 
    (IloNumToNumSegmentFunctionI*) (fct1.getImpl()->makeCopy());
  fct->setAdd(fct2.getImpl());
  return (IloNumToNumSegmentFunction)fct;
}

IloNumToNumSegmentFunction operator-(const IloNumToNumSegmentFunction fct1,
                                     const IloNumToNumSegmentFunction fct2){
  IloNumToNumSegmentFunctionI* fct = (IloNumToNumSegmentFunctionI*)(fct1.getImpl()->makeCopy());
  fct->setSub(fct2.getImpl());
  return (IloNumToNumSegmentFunction)fct;
}

IloNumToNumSegmentFunction IloMin(const IloNumToNumSegmentFunction fct1,
                                  const IloNumToNumSegmentFunction fct2){
  IloNumToNumSegmentFunctionI* fct = (IloNumToNumSegmentFunctionI*)(fct1.getImpl()->makeCopy());
  fct->setMin(fct2.getImpl());
  return (IloNumToNumSegmentFunction)fct;
}

IloNumToNumSegmentFunction IloMax(const IloNumToNumSegmentFunction fct1,
                                  const IloNumToNumSegmentFunction fct2){
  IloNumToNumSegmentFunctionI* fct = (IloNumToNumSegmentFunctionI*)(fct1.getImpl()->makeCopy());
  fct->setMax(fct2.getImpl());
  return (IloNumToNumSegmentFunction)fct;
}

//-------------------------------------------------------
// class IloNumToNumSegmentFunctionCursor
//-------------------------------------------------------

IloNumToNumSegmentFunctionCursor::
IloNumToNumSegmentFunctionCursor(const IloNumToNumSegmentFunction fct)
  : _function (fct.getImpl())
{
  assert(_function != 0);
  IloNum x = _function->getXMin();
  _cursor = (IloFunctionSegmentI*)(_function->seek(x));
  assert(_cursor !=0);
}

IloNumToNumSegmentFunctionCursor::
IloNumToNumSegmentFunctionCursor(const IloNumToNumSegmentFunction fct, IloNum x)
  : _function (fct.getImpl())
{
  assert(_function != 0);
  if ((x < _function->getXMin()) ||
      (x >= _function->getXMax()))
    throw(IloParameterI::Exception(
				   "Cannot create cursor out of function definition interval"));

  _cursor = (IloFunctionSegmentI*)(_function->seek(x));
  assert(_cursor !=0);
}

IloNumToNumSegmentFunctionCursor::
IloNumToNumSegmentFunctionCursor(const IloNumToNumSegmentFunctionCursor& curs)
  : _function(curs._function),
    _cursor(curs._cursor)
{}  

IloNumToNumSegmentFunctionCursor::
IloNumToNumSegmentFunctionCursor()
  : _function(0),
    _cursor(0)
{}  

IloNumToNumSegmentFunctionCursor::~IloNumToNumSegmentFunctionCursor() {}

void 
IloNumToNumSegmentFunctionCursor::operator=(const IloNumToNumSegmentFunctionCursor& csor) {
  _function = csor._function;
  _cursor   = csor._cursor;
}

void IloNumToNumSegmentFunctionCursor::seek(IloNum x) {
  if ((x < _function->getXMin()) ||
      (x >= _function->getXMax()))
    throw(IloParameterI::Exception(
				   "Cannot create cursor outside of function definition interval"));
  _cursor = (IloFunctionSegmentI*)(_function->seek(x));
  assert(_cursor !=0);  
}

