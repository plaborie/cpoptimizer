// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------



// This file contains the IluEventI class, the IluActEventI class, the IluResourceEventI class, the IluFailEventI class,
// the IluActStartEventI class, the IluActEndEventI class, the IluResourceStartEventI class, and the IluResourceEndEventI class.



#ifndef __SIM_ilueventsimH
#define __SIM_ilueventsimH

#include <ilsolver/ilcerr.h>

#include "iluactivity.h"
#include "iluunaryresource.h"


class IluDynamicSchedulerI;
class IluTimeEventManagerI;
class IluRealizationI;


// --------------------------------------------------------------------------
// THIS   ROOT   CLASS  ALLOWS  US  TO  BUILD  EVENTS  THAT  CORRESPOND  TO 
// ACTIVITY START AND END TIMES, RESOURCE BREAKDOWN START AND END TIMES.
// --------------------------------------------------------------------------


class IluEventI {
private:
	IloInt		_time;		// _time is the date at which the event occurs.
	IluEventI*	_next;
	IloBool		_suspended;	//_suspended indicates whether the event is currently suspended (IloTrue) or not (IloFalse).

protected: // This class contains a pure virtual member function.
	IluEventI() : _time(IlcIntMax), _next(0), _suspended(IloFalse) {}
	IluEventI(IloInt time) : _time(time), _next(0), _suspended(IloFalse) {}

public:
	~IluEventI() {}

	void setTime(IloInt t) { _time = t; }
	IloInt getTime() const { return _time; }

	void setNext(IluEventI* eventP) { _next = eventP; }
	IluEventI* getNext() const { return _next; }

	void setSuspended(IloBool suspended) { _suspended = suspended; }
	IloBool isSuspended() const { return _suspended; }

// execute is a pure virtual memberfunction that characterizes the event type used: we cannot instantiate this class.
	virtual void execute(IluDynamicSchedulerI*, IluTimeEventManagerI*) = 0;
};



class IluActEventI:  public IluEventI { // We cannot instantiate this class.
private:
	IluActivityI* _uncActivityP;

protected:	
	IluActEventI(): IluEventI(), _uncActivityP(0) {}
	IluActEventI(IloInt time, IluActivityI* uncActivityP) : IluEventI(time), _uncActivityP(uncActivityP) {}

public:
	~IluActEventI() {}
	IluActivityI* getUncActivity() { return _uncActivityP; }
	void setUncActivity(IluActivityI* uncActivityP) { _uncActivityP = uncActivityP; }
}; 



class IluResourceEventI:	public IluEventI{// We cannot instantiate this class.
private:
	IluUnaryResourceI* _uncResourceP;

protected:
	IluResourceEventI(): IluEventI(), _uncResourceP(0) {}
	IluResourceEventI(IlcInt time, IluUnaryResourceI* uncResourceP): IluEventI(time), _uncResourceP(uncResourceP){}

public:
	~IluResourceEventI() {}
	IluUnaryResourceI* getUncUnaryResource() { return _uncResourceP; }
	void setUncUnaryResource(IluUnaryResourceI* uncResourceP) { _uncResourceP = uncResourceP; }
};



class IluActStartEventI:  public IluActEventI {
public:
	IluActStartEventI() : IluActEventI() {}
	IluActStartEventI(IlcInt time, IluActivityI* uncActivityP) : IluActEventI(time, uncActivityP) {}
	~IluActStartEventI() {}
	
	void execute(IluDynamicSchedulerI*, IluTimeEventManagerI*);
};



class IluActEndEventI:    public IluActEventI {
public:
	IluActEndEventI() : IluActEventI() {}
	IluActEndEventI(IloInt time, IluActivityI* uncActivityP) : IluActEventI(time, uncActivityP) {}
	~IluActEndEventI() {}

	void execute(IluDynamicSchedulerI*, IluTimeEventManagerI*);
};



class IluResourceStartEventI:  public IluResourceEventI {
public:
	IluResourceStartEventI(): IluResourceEventI() {}
	IluResourceStartEventI(IloInt time, IluUnaryResourceI* uncResourceP) : IluResourceEventI(time, uncResourceP) {}
	~IluResourceStartEventI() {}

	void execute(IluDynamicSchedulerI*, IluTimeEventManagerI*);
};



class IluResourceEndEventI:  public IluResourceEventI {
public:
	IluResourceEndEventI(): IluResourceEventI() {}
	IluResourceEndEventI(IloInt time, IluUnaryResourceI* uncResourceP) : IluResourceEventI(time, uncResourceP) {}
	~IluResourceEndEventI() {}

	void execute(IluDynamicSchedulerI*, IluTimeEventManagerI*);
};



/*class IluFailEventI: public IluEventI {
public:
	IluFailEventI() : IluEventI() {}
	IluFailEventI(IloInt time) : IluEventI(time) {}
	~IluFailEventI() {}
	
	void execute(IluDynamicSchedulerI*, IluTimeEventManagerI*);
};*/
#endif
